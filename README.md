# TP/AEDSIII - Sistema de Matrícula Acadêmica

**Trabalho Prático da Disciplina AEDS III** - Pontifícia Universidade Católica de Minas Gerais (PUC-MG)

---

## 1. Resumo Executivo

Sistema acadêmico de gerenciamento de matrículas em **C++20** com arquitetura **MVC** baseada em *flat namespaces*. Implementa persistência binária em arquivo único (`records.dat`) com **chunks de tamanho fixo** para 4 entidades (Student 139B, Teacher 165B, Subject 84B), **índice hash persistente** (`records.idx`) para buscas O(1) por ID e nome, e **Árvore B+** dentro do próprio arquivo como estrutura central de matrículas — dispensando qualquer EnrollmentRecord separado. Interface gráfica via **Lua/ImGui** com 10+ views e crescimento dinâmico automático dos chunks.

**Palavras-chave**: C++20, MVC, Persistência Binária, Chunks, B+ Tree, Hash Index, Lua DSL, Dear ImGui, PUC-MG

---

## 2. Introdução

### 2.1 Contextualização e Motivação

Trabalho prático da disciplina Algoritmos e Estruturas de Dados III (AEDS III) da PUC-MG. O projeto implementa um sistema completo de matrículas com persistência binária de baixo nível, indexação hash/árvore B+, crescimento dinâmico de armazenamento, e interface gráfica com scripting Lua.

### 2.2 Objetivos do Projeto

- Implementar CRUD completo para Alunos, Professores e Disciplinas
- Gerenciar matrículas via Árvore B+ com consultas por prefixo
- Indexar todos os registros para busca O(1)
- Suportar crescimento dinâmico do arquivo (chunks duplicam quando cheios)
- Interface gráfica separada em Lua (DSL) sem recompilação
- Cobertura de testes unitários

### 2.3 Escopo Funcional

- Cadastro de alunos (nome, email, cpf, data nascimento, curso, ano)
- Cadastro de professores (nome, email, cpf, departamento, especialização, contratação)
- Cadastro de disciplinas (nome, código, créditos, professor responsável)
- Matrícula de alunos em disciplinas com notas e semestre
- Consulta de matrículas por aluno ou por disciplina
- Remoção lógica (soft delete) de registros
- Índice hash persistente para busca por ID ou nome
- Crescimento automático do arquivo

---

## 3. Fundamentação Teórica

### 3.1 Arquitetura MVC

| Camada | Namespace | Componentes |
|--------|-----------|-------------|
| Utility | `project_utility` | Constants.hpp, Enums.hpp |
| Model | `project_model` | Record.hpp (UserRecord, StudentRecord, TeacherRecord, SubjectRecord, FileHeader, ChunkInfo), BPlusTree.hpp/cpp |
| Controller | `project_controller` | DataManager, FileManager, IndexCtrl |
| View | `project_view` | ImguiBindings.cpp + scripts Lua |

### 3.2 Persistência com Chunks

Arquivo único `records.dat` com layout:

```
[File Header 256B] → [Chunk Aluno] → [Chunk Professor] → [Chunk Disciplina] → [Chunk B+ Tree]
```

- Cada entidade tem seu próprio chunk com registros de tamanho fixo
- FileHeader incorpora 4 `ChunkInfo` (offset, recordSize, capacity, used)
- Quando `used >= capacity`, o chunk dobra de tamanho e o arquivo é reescrito

### 3.3 Índice Hash

Arquivo separado `records.idx` com chaves:
- `STU:<id>` → Aluno por ID
- `TCH:<id>` → Professor por ID
- `SUB:<id>` → Disciplina por ID
- `NM:<nome>` → Busca por nome

Usa hash DJB2 com tabela dinâmica.

### 3.4 Árvore B+ para Matrículas

A matrícula **não** possui struct separada — é armazenada diretamente como entrada na B+ Tree:

- **Chave**: `ENR:STU:0042:SUB:0015` (20 bytes, zero-padded)
- **Valor**: `{studentId, subjectId, teacherId, grade, semester}` (28 bytes)

Isso permite consultas por prefixo:
- `ENR:STU:0042:` → todas as matrículas de um aluno
- `ENR:STU:*:SUB:0015` → todos os alunos de uma disciplina

A B+ Tree usa páginas de 4096 bytes (~84 entries/leaf, ~145 keys/internal).

---

## 4. Arquitetura do Sistema

### 4.1 Visão Geral

```
┌──────────────────────────────────────────────────────────────────┐
│                        TP_AEDSIII                                │
├──────────────────────────────────────────────────────────────────┤
│  C++ Backend              │  Lua Views (DSL)    │  ImGui        │
│  ┌────────────────────┐   │  ┌───────────────┐   │  ┌────┐      │
│  │ project_utility    │   │  │ router.lua    │───│─›│ UI │      │
│  │ (Constants, Enums) │   │  │ common.lua    │   │  └────┘      │
│  ├────────────────────┤   │  │ MainMenu      │   │              │
│  │ project_model      │   │  │ StudentCreate │   │              │
│  │ (Record, BPlusTree)│   │  │ StudentList   │   │              │
│  ├────────────────────┤   │  │ StudentDetail │   │              │
│  │ project_controller │   │  │ TeacherCreate │   │              │
│  │ (DataManager,      │   │  │ TeacherList   │   │              │
│  │  FileManager,      │   │  │ SubjectCreate │   │              │
│  │  IndexCtrl)        │   │  │ SubjectList   │   │              │
│  ├────────────────────┤   │  │ EnrollmentList│   │              │
│  │ project_view       │   │  └───────────────┘   │              │
│  │ (ImguiBindings)    │   │                      │              │
│  └────────────────────┘   │  4-Cores Theme:       │              │
│                           │  Preto, Branco,       │              │
│                           │  Verde, Vermelho      │              │
└──────────────────────────────────────────────────────────────────┘
```

### 4.2 Stack Tecnológico

| Componente | Tecnologia | Propósito |
|------------|-----------|-----------|
| Linguagem | C++20 | Backend e lógica de negócio |
| Build | CMake 3.20+ + Ninja | Build cross-platform |
| GUI | Dear ImGui (hello_imgui v1.5.0) | Renderização imediata |
| Scripting | Lua 5.x | DSL para definições de UI |
| Bindings | ruaaa | Integração C++/Lua |
| Índice | DJB2 Hash + B+ Tree | Busca O(1) e range scan |

---

## 5. Especificações Técnicas

### 5.1 Formato Binário — `records.dat`

**File Header (256 bytes):**

| Offset | Tamanho | Campo |
|--------|---------|-------|
| 0 | 4 | magic (0x52454331) |
| 4 | 4 | version |
| 8 | 4 | headerSize |
| 12 | 4 | chunkCount |
| 16 | 12 | nextStudentId, nextTeacherId, nextSubjectId (3×4B) |
| 28 | 4 | padding |
| 32 | 128 | ChunkInfo[4] (4×32B) |
| 160 | 96 | reserved |

**ChunkInfo (32 bytes cada):**

| Offset | Tamanho | Campo |
|--------|---------|-------|
| 0 | 1 | type ('S', 'T', 'B', 'I') |
| 1 | 3 | padding |
| 4 | 8 | offset (uint64_t) |
| 12 | 4 | recordSize (uint32_t) |
| 16 | 4 | capacity (uint32_t) |
| 20 | 4 | used (uint32_t) |
| 24 | 8 | padding |

**StudentRecord (139 bytes):**

| Offset | Tamanho | Campo |
|--------|---------|-------|
| 0 | 1 | status |
| 1 | 1 | type ('S') |
| 2 | 4 | userId |
| 6 | 50 | name |
| 56 | 30 | email |
| 86 | 15 | cpf |
| 101 | 4 | birthDate |
| 105 | 30 | courseName |
| 135 | 4 | enrollmentYear |

**TeacherRecord (165 bytes):**

| Offset | Tamanho | Campo |
|--------|---------|-------|
| 0 | 1 | status |
| 1 | 1 | type ('T') |
| 2 | 4 | userId |
| 6 | 50 | name |
| 56 | 30 | email |
| 86 | 15 | cpf |
| 101 | 30 | department |
| 131 | 30 | specialization |
| 161 | 4 | hireDate |

**SubjectRecord (84 bytes):**

| Offset | Tamanho | Campo |
|--------|---------|-------|
| 0 | 1 | status |
| 1 | 1 | type ('B') |
| 2 | 4 | subjectId |
| 6 | 50 | name |
| 56 | 20 | code |
| 76 | 4 | credits |
| 80 | 4 | teacherId |

**B+ Tree Leaf Entry (28 bytes):** studentId + subjectId + teacherId + grade (float) + semester[12]

### 5.2 Árvore B+ — Páginas de 4096B

| Parâmetro | Valor |
|-----------|-------|
| Tamanho da página | 4096 bytes |
| Tamanho da chave | 20 bytes |
| Tamanho do valor (folha) | 28 bytes |
| Entradas por folha | ~84 |
| Chaves por interno | ~144 (fanout ~145) |
| Formato da chave | `ENR:STU:NNNN:SUB:MMMM` |

### 5.3 Complexidade das Operações

| Operação | Complexidade | Estrutura |
|----------|-------------|-----------|
| Create (qualquer entidade) | O(1) | Append no chunk + insert no hash |
| Read by ID | O(1) | Lookup no hash index |
| Delete | O(1) | Hash lookup + soft delete |
| List all | O(n) | Scan chunk filtrando ativos |
| Matricular | O(log N) | Insert na B+ Tree |
| Buscar matrículas (por aluno) | O(log N + K) | Range scan por prefixo na B+ Tree |
| Atualizar nota | O(log N) | Erase + insert na B+ Tree |
| Reallocar chunk | O(n) | Reescreve arquivo com capacidade dobrada |

---

## 6. Interface Gráfica

### 6.1 Views em Lua DSL

```
src/view/scripts/views/
├── router.lua            # Navegação
├── common.lua            # Sidebar + atalhos
├── MainMenu.lua          # Menu principal com stats
├── StudentCreate.lua     # Cadastro de aluno
├── StudentList.lua       # Listar alunos
├── StudentDetail.lua     # Consultar aluno por ID
├── TeacherCreate.lua     # Cadastro de professor
├── TeacherList.lua       # Listar professores
├── SubjectCreate.lua     # Cadastro de disciplina
├── SubjectList.lua       # Listar disciplinas
└── EnrollmentList.lua    # Matrículas (CRUD + notas + consultas)
```

### 6.2 Tema Minimalista (4 Cores)

| Cor | Uso |
|-----|-----|
| **Preto** | Backgrounds, janelas |
| **Branco** | Textos, elementos ativos |
| **Verde** | Sucesso, confirmações, botões positivos |
| **Vermelho** | Erros, alertas, exclusões |

---

## 7. Testes Unitários

| # | Teste | Funcionalidade Validada |
|---|-------|------------------------|
| 1 | test_initialize | Inicialização do DataManager |
| 2 | test_create_student | Criação de aluno (6 campos) |
| 3 | test_read_student | Leitura por ID via hash |
| 4 | test_read_nonexistent | Tratamento de ID inexistente |
| 5 | test_list_all_students | Listagem de alunos ativos |
| 6 | test_soft_delete | Soft delete com verificação de contagem |
| 7 | test_create_teacher_and_subject | CRUD professor + disciplina |
| 8 | test_enrollment | Matrícula via B+ Tree |
| 9 | test_update_grade | Atualização de nota |
| 10 | test_enrollments_by_student | Consulta de matrículas por aluno |

---

## 8. Instruções de Build

### 8.1 Pré-requisitos

#### Linux (Debian/Ubuntu)
```bash
sudo apt update
sudo apt install cmake ninja-build gcc g++ libglfw3-dev ccache
```

#### macOS
```bash
brew update
brew install cmake ninja glfw ccache
```

### 8.2 Compilação

```bash
# Limpar build anterior
rm -rf cmake-build-release_build

# Configurar e compilar com Ninja
cmake -G Ninja -S . -B cmake-build-release_build
cmake --build cmake-build-release_build

# Executar GUI
./builds/TP_AEDSIII

# Executar testes
./builds/run_tests
```

**Nota**: As dependências (hello_imgui, lua, luaaa) estão pré-clonadas em `libs/`. Nenhuma instalação de sistema ou download é necessária além do GLFW.

### 8.3 Solução de Problemas

| Erro | Solução |
|------|---------|
| `CMAKE_GENERATOR` not found | `rm -rf cmake-build-release_build && cmake -G Ninja -S . -B cmake-build-release_build` |
| GLFW não encontrado | `sudo apt install libglfw3-dev` (Linux) ou `brew install glfw` (macOS) |

---

## 9. Diagramas UML

| Diagrama | Descrição |
|----------|-----------|
| ![Arquitetura](./docs/UML%20Diagrams/arquitetura_sistema.png) | Arquitetura geral do sistema |
| ![Classes](./docs/UML%20Diagrams/diagrama_classes.png) | Diagrama de classes UML |
| ![Sequência CRUD](./docs/UML%20Diagrams/diagrama_sequencia_crud.png) | Sequência de operações CRUD |
| ![Estrutura Binária](./docs/UML%20Diagrams/estrutura_binaria.png) | Estrutura binária do arquivo único |

---

## 10. Estrutura de Diretórios

```
TP/
├── src/
│   ├── main.cpp                    # Entry point + bindings Lua
│   ├── utility/                    # project_utility
│   │   ├── Constants.hpp           # Constantes (record sizes, chunk defaults)
│   │   └── Enums.hpp               # RecStatus, RecType, ViewId
│   ├── model/                      # project_model
│   │   ├── Record.hpp              # UserRecord, StudentRecord, TeacherRecord,
│   │   │                           # SubjectRecord, BTreeLeafValue, FileHeader
│   │   ├── Record.cpp              # Serialização binária (toBytes/fromBytes)
│   │   ├── BPlusTree.hpp           # B+ Tree header (PAGE_SIZE=4096, KEY_SIZE=20)
│   │   └── BPlusTree.cpp           # Insert/split, search/rangeScan, erase/merge
│   ├── controller/                 # project_controller
│   │   ├── DataManager.hpp         # CRUD exposto para Lua + chunk management
│   │   ├── DataManager.cpp         # Implementação com chunks + B+ tree
│   │   ├── FileManager.hpp         # I/O binário genérico (recordSize param)
│   │   ├── FileManager.cpp         # Read/write/append/markDeleted
│   │   ├── IndexCtrl.hpp           # Hash index persistente (DJB2)
│   │   └── IndexCtrl.cpp           # Insert/lookup/remove/nextId
│   ├── view/                       # project_view
│   │   ├── bindings/ImguiBindings.cpp  # Bindings ImGui para Lua
│   │   └── scripts/views/          # Lua DSL views
│   │       ├── router.lua          # Navegação
│   │       ├── common.lua          # Sidebar + helpers
│   │       ├── MainMenu.lua        # Menu principal
│   │       ├── StudentCreate.lua   # Cadastro aluno
│   │       ├── StudentList.lua     # Listar alunos
│   │       ├── StudentDetail.lua   # Consultar aluno
│   │       ├── TeacherCreate.lua   # Cadastro professor
│   │       ├── TeacherList.lua     # Listar professores
│   │       ├── SubjectCreate.lua   # Cadastro disciplina
│   │       ├── SubjectList.lua     # Listar disciplinas
│   │       └── EnrollmentList.lua  # Gerenciar matrículas
├── libs/                           # Dependências pré-clonadas
│   ├── hello_imgui/                # Hello ImGui v1.5.0
│   ├── lua/                        # Lua 5.4.8
│   └── luaaa/                      # ruaaa C++/Lua bindings
├── builds/                         # Executáveis gerados
│   ├── TP_AEDSIII                  # GUI (~22MB)
│   └── run_tests                   # Testes (~2.8MB)
├── data/                           # Dados de runtime
├── tests/
│   └── test_main.cpp               # 10 testes unitários
└── docs/
    ├── Markdown/                   # Documentação em markdown
    ├── Text/                       # Documentação em texto puro
    ├── UML Diagrams/               # Diagramas PlantUML + PNG
    └── ux/                         # Specs e mockups
```

---

## 11. Diferenciais do Projeto

1. **Arquivo único com chunks**: 4 entidades no mesmo arquivo com layout determinístico
2. **B+ Tree como armazenamento de matrícula**: Sem EnrollmentRecord separado — a chave composta `ENR:STU:NNNN:SUB:MMMM` é o relacionamento
3. **Índice hash persistente**: Atualizado sincronamente, sem rebuilds periódicos
4. **UserRecord com herança**: StudentRecord e TeacherRecord herdam campos comuns (name, email, cpf)
5. **Crescimento dinâmico**: Chunks duplicam quando cheios (amortized O(1))
6. **B+ Tree com 4096B páginas**: ~84 entries/leaf, prefix scan para consultas por aluno/disciplina
7. **IDs auto-incremento por tipo**: `padId()` com zero-padding para chaves ordenáveis
8. **Lua DSL**: 10+ views modificáveis sem recompilação C++
9. **Build offline**: Dependências em `libs/`, sem FetchContent downloads
10. **Soft delete**: Status byte `'*'` mantém dados recuperáveis

---

## 12. Conclusão

O projeto TP/AEDSIII evoluiu de um sistema de registro único (67 bytes) para uma arquitetura completa com 4 entidades, índice hash persistente, Árvore B+ para matrículas, e crescimento dinâmico de armazenamento. A B+ Tree substitui a necessidade de um EnrollmentRecord separado — a própria chave na árvore é o vínculo entre aluno e disciplina.

O sistema está funcional, com build verificado e testes unitários, pronto para uso e extensão.

---

## Referências

- Hello ImGui: https://github.com/pthom/hello_imgui
- Lua: https://www.lua.org/docs.html
- Dear ImGui: https://github.com/ocornut/imgui
- ruaaa: https://github.com/gengyong/luaaa
- PlantUML: https://plantuml.com

---

*Documento preparado para a disciplina AEDS III - PUC-MG*  
*Para dúvidas técnicas, consulte `AGENTS.md`*

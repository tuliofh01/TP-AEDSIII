# TP/AEDSIII — Sistema de Matrícula Acadêmica

**Trabalho Prático da Disciplina AEDS III** — Pontifícia Universidade Católica de Minas Gerais (PUC-MG)

---

## 1. Resumo Executivo

Sistema acadêmico de gerenciamento de matrículas em **C++20** com arquitetura **MVC** em *flat namespaces*. Implementa persistência binária em arquivo único (`records.dat`) com **chunks de tamanho fixo** para 4 entidades (Student 141B, Teacher 167B, Subject 84B), **índice hash persistente** (`records.idx`) para buscas O(1) por CPF, e **Árvore B+** para armazenamento central de matrículas com chave composta e consultas por prefixo. Interface gráfica via **Lua/ImGui** scripting com 10 views e crescimento dinâmico automático dos chunks.

**Palavras-chave**: C++20, MVC, Persistência Binária, Chunks, B+ Tree, Hash Index, Lua DSL, Dear ImGui, PUC-MG

---

## 2. Introdução

### 2.1 Contextualização e Motivação

Trabalho prático da disciplina Algoritmos e Estruturas de Dados III (AEDS III) da PUC-MG. O projeto implementa um sistema completo de matrículas com persistência binária de baixo nível, indexação hash/árvore B+, crescimento dinâmico de armazenamento, autenticação por CPF/senha, e interface gráfica com scripting Lua.

### 2.2 Objetivos do Projeto

- Implementar CRUD completo para Alunos, Professores e Disciplinas
- Gerenciar matrículas via Árvore B+ com consultas por prefixo
- Indexar todos os registros para busca O(1) por CPF
- Suportar crescimento dinâmico do arquivo (chunks duplicam quando cheios)
- Interface gráfica em Lua (DSL) sem recompilação
- Autenticação por CPF + senha numérica com views por papel (aluno/professor)
- Cobertura de testes unitários (11 testes)

### 2.3 Escopo Funcional

- Cadastro de alunos (nome, email, cpf, senha, data nascimento, curso, ano)
- Cadastro de professores (nome, email, cpf, senha, departamento, especialização, contratação)
- Cadastro de disciplinas (nome, código, créditos, professor responsável)
- Matrícula de alunos em disciplinas com notas e semestre
- Consulta de matrículas por aluno ou por disciplina
- Remoção lógica (soft delete) de registros
- Índice hash persistente para busca por CPF
- Autenticação com diferenciação de views (S=aluno, T=professor)
- Grade escala inteira 0–100 (uint8_t)
- Crescimento automático do arquivo

---

## 3. Fundamentação Teórica

### 3.1 Arquitetura MVC

| Camada | Namespace | Componentes |
|--------|-----------|-------------|
| Utility | `project_utility` | Constants.hpp, Enums.hpp |
| Model | `project_model` | Record.hpp (UserRecord, StudentRecord, TeacherRecord, SubjectRecord, BTreeLeafValue, FileHeader, ChunkInfo), Student.hpp, Teacher.hpp, Subject.hpp, Enrollment.hpp, BPlusTree.hpp/cpp |
| Controller | `project_controller` | DataManager, FileManager, IndexCtrl |
| View | `project_view` | ImguiBindings.cpp, ScriptArchive.hpp/cpp + scripts Lua |

### 3.2 Persistência com Chunks

Arquivo único `records.dat` com layout:

```
[File Header 256B] → [Chunk Aluno] → [Chunk Professor] → [Chunk Disciplina] → [Chunk B+ Tree]
```

- Cada entidade tem seu próprio chunk com registros de tamanho fixo
- FileHeader incorpora 4 `ChunkInfo` (offset, recordSize, capacity, used)
- Quando `used >= capacity`, o chunk dobra de tamanho e o arquivo é reescrito
- Chunk type 'I' reservado para páginas da B+ Tree

### 3.3 Índice Hash

Arquivo separado `records.idx` com chaves:
- `CPF:<cpf>` → busca de login por CPF
- `STU:<id>` → aluno por ID (fallback)
- `TCH:<id>` → professor por ID
- `SUB:<id>` → disciplina por ID

Usa hash DJB2 com tabela dinâmica e rebuild automático a cada N operações.

### 3.4 Árvore B+ para Matrículas

Matrícula armazenada como entrada na B+ Tree com chave composta:

- **Chave**: `ENR:STU:0001:SUB:0001` (20 bytes, zero-padded)
- **Valor**: `BTreeLeafValue{studentId, subjectId, teacherId, grade (uint8_t), semester}` (25 bytes)

Isso permite consultas por prefixo:
- `ENR:STU:0001:` → todas as matrículas de um aluno
- `ENR:STU:*:SUB:0001` → todos os alunos de uma disciplina

A B+ Tree usa páginas de 4096 bytes (~89 entries/leaf, ~145 keys/internal).

---

## 4. Arquitetura do Sistema

### 4.1 Visão Geral

A arquitetura segue MVC com **C++** no backend (model + controller) e **Lua scripts** como DSL de views renderizadas via Dear ImGui. Diagramas completos em `docs/UML Diagrams/`:

| Diagrama | Descrição | Arquivo |
|----------|-----------|---------|
| Arquitetura do Sistema | Visão geral dos componentes e fluxo de dados | [`docs/UML Diagrams/arquitetura_sistema.png`](docs/UML%20Diagrams/arquitetura_sistema.png) |
| Diagrama de Classes | Estrutura de classes C++ e relacionamentos | [`docs/UML Diagrams/diagrama_classes.png`](docs/UML%20Diagrams/diagrama_classes.png) |
| Estrutura Binária | Layout do arquivo `records.dat` (chunks + header) | [`docs/UML Diagrams/estrutura_binaria.png`](docs/UML%20Diagrams/estrutura_binaria.png) |
| Diagrama de Sequência | Fluxo CRUD (Create, Read, Update, Delete) | [`docs/UML Diagrams/diagrama_sequencia_crud.png`](docs/UML%20Diagrams/diagrama_sequencia_crud.png) |
| Diagrama de Relacionamento | Entidades e seus relacionamentos | [`docs/UML Diagrams/diagrama_relacionamento.png`](docs/UML%20Diagrams/diagrama_relacionamento.png) |
| Diagrama de Casos de Uso | Atores e funcionalidades do sistema | [`docs/UML Diagrams/diagrama_casos_uso.png`](docs/UML%20Diagrams/diagrama_casos_uso.png) |
| Fluxograma | Fluxo de execução do sistema | [`docs/UML Diagrams/fluxograma.png`](docs/UML%20Diagrams/fluxograma.png) |

Fontes editáveis (`.puml` para PlantUML, `.mmd` para Mermaid) em `docs/UML Diagrams/src/`.

### 4.2 Stack Tecnológico

| Componente | Tecnologia | Propósito |
|------------|-----------|-----------|
| Linguagem | C++20 | Backend e lógica de negócio |
| Build | CMake 3.20+ | Build cross-platform |
| GUI | Dear ImGui (hello_imgui v1.5.0) | Renderização imediata |
| Scripting | Lua 5.x | DSL para definições de UI |
| Bindings | luaaa | Integração C++/Lua |
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

**UserRecord — campos comuns (base para StudentRecord e TeacherRecord):**

| Offset | Tamanho | Campo |
|--------|---------|-------|
| 0 | 1 | status ('A' ou '*') |
| 1 | 1 | type ('S' ou 'T') |
| 2 | 4 | userId (int32_t) |
| 6 | 2 | password (uint16_t) |
| 8 | 50 | name |
| 58 | 30 | email |
| 88 | 15 | cpf |
| 103 | — | (fim do UserRecord, campos específicos a seguir) |

**StudentRecord (141 bytes = UserRecord + birthDate + courseName + enrollmentYear):**

| Offset | Tamanho | Campo |
|--------|---------|-------|
| 103 | 4 | birthDate |
| 107 | 30 | courseName |
| 137 | 4 | enrollmentYear |

**TeacherRecord (167 bytes = UserRecord + department + specialization + hireDate):**

| Offset | Tamanho | Campo |
|--------|---------|-------|
| 103 | 30 | department |
| 133 | 30 | specialization |
| 163 | 4 | hireDate |

**SubjectRecord (84 bytes, independente — não herda UserRecord):**

| Offset | Tamanho | Campo |
|--------|---------|-------|
| 0 | 1 | status |
| 1 | 1 | type ('B') |
| 2 | 4 | subjectId |
| 6 | 50 | name |
| 56 | 20 | code |
| 76 | 4 | credits |
| 80 | 4 | teacherId |

**B+ Tree Leaf Entry — BTreeLeafValue (25 bytes):**

| Offset | Tamanho | Campo |
|--------|---------|-------|
| 0 | 4 | studentId |
| 4 | 4 | subjectId |
| 8 | 4 | teacherId |
| 12 | 1 | grade (uint8_t, 0–100) |
| 13 | 12 | semester |

### 5.2 Árvore B+ — Páginas de 4096B

| Parâmetro | Valor |
|-----------|-------|
| Tamanho da página | 4096 bytes |
| Tamanho da chave | 20 bytes |
| Tamanho do valor (folha) | 25 bytes |
| Entradas por folha | ~89 |
| Chaves por interno | ~145 (fanout ~146) |
| Formato da chave | `ENR:STU:NNNN:SUB:MMMM` |

### 5.3 Complexidade das Operações

| Operação | Complexidade | Estrutura |
|----------|-------------|-----------|
| Create (qualquer entidade) | O(1) | Append no chunk + insert no hash |
| Read by ID | O(1) | Lookup no hash index |
| Delete | O(1) | Hash lookup + soft delete |
| List all | O(n) | Scan chunk filtrando ativos |
| Login (CPF + senha) | O(1) | Hash lookup por CPF |
| Matricular | O(log N) | Insert na B+ Tree |
| Buscar matrículas (por aluno) | O(log N + K) | Range scan por prefixo na B+ Tree |
| Atualizar nota | O(log N) | Erase + insert na B+ Tree |
| Reallocar chunk | O(n) | Reescreve arquivo com capacidade dobrada |

---

## 6. Autenticação e Controle de Acesso

Sistema de login com dois papéis:

| Papel | Tipo | Acesso |
|-------|------|--------|
| Aluno | `role == 'S'` | Meus Dados, Minhas Matrículas |
| Professor | `role == 'T'` | CRUD completo (alunos, professores, disciplinas, matrículas) |

- Autenticação via `login(cpf, password)` — busca O(1) no índice hash por CPF
- Senha armazenada como `uint16_t` no `password` field do `UserRecord`
- Resultado do login (`LoginResult`) exposto como tabela Lua: `{userId, role, name}`
- View Router (`router.lua`) redireciona baseado em `user.role`

### 6.1 Credenciais de Amostra

Dados populados automaticamente na primeira execução (quando o banco está vazio):

| Papel | CPF | Senha |
|-------|-----|-------|
| Professor | `00000000000` | `1234` |
| Aluno | `11111111111` | `1234` |
| Aluno | `22222222222` | `1234` |
| Aluno | `33333333333` | `1234` |
| Aluno | `44444444444` | `1234` |
| Aluno | `55555555555` | `1234` |

Delete `data/records.dat` para resetar e repovoar.

---

## 7. Interface Gráfica

### 7.1 Views em Lua DSL

```
src/view/scripts/
├── handlers/
│   ├── router.lua            # Navegação + autenticação
│   └── common.lua            # Sidebar + helpers + tema 4 cores
├── GUI/
│   ├── functional_views/     # Views CRUD (professor) + login
│   │   ├── MainMenu.lua      # Menu principal com estatísticas
│   │   ├── LoginView.lua     # Tela de login
│   │   ├── StudentList.lua   # Listar alunos
│   │   ├── TeacherCreate.lua # Cadastro de professor
│   │   ├── TeacherList.lua   # Listar professores
│   │   ├── SubjectCreate.lua # Cadastro de disciplina
│   │   └── SubjectList.lua   # Listar disciplinas
│   ├── displayer_views/      # Views de exibição (aluno + professor)
│   │   ├── StudentCreate.lua # Cadastro de aluno
│   │   ├── StudentDetail.lua # Consultar aluno por ID
│   │   ├── StudentProfile.lua# Perfil do aluno logado
│   │   └── EnrollmentList.lua# Matrículas (CRUD + notas + consultas)
│   └── misc/
│       ├── globals.lua       # Constantes globais Lua
│       └── populate_samples.lua  # Povoamento de dados de exemplo
```

### 7.2 Navegação por Sidebar

Barra lateral esquerda (150px) com menu contextual conforme papel do usuário:

- **Aluno (`'S'`)**: Meus Dados, Minhas Matrículas, Sair
- **Professor (`'T'`)**: Seções de Estudantes (Cadastrar, Listar, Consultar), Professores (Cadastrar, Listar), Disciplinas (Cadastrar, Listar), Matrículas (Gerenciar), Sair
- Botão ativo destacado em verde; mouseover com feedback visual

### 7.3 Tema Minimalista (4 Cores)

| Cor | Uso |
|-----|-----|
| **Preto** | Backgrounds, janelas |
| **Branco** | Textos, elementos ativos |
| **Verde** | Sucesso, confirmações, botões positivos, seções do menu |
| **Vermelho** | Erros, alertas, botão "Sair" |

- Todas as cores usam PushStyleColor/PopStyleColor balanceados
- Popups modais para feedback de operações (sucesso/erro)

### 7.4 ScriptArchive

Os 15 scripts `.lua` são empacotados em `builds/compiled_executable/scripts.bin` via ferramenta `pack_scripts` durante o build. Em runtime, o `ScriptArchive` carrega o binário e registra todos os scripts no estado Lua via `luaL_loadbuffer` — eliminando dependência de sistema de arquivos.

---

## 8. Modelo de Dados — Wrappers vs. Records

Separação explícita entre **packed structs binárias** (`*Record`) e **modelos C++** (`Student`, `Teacher`, `Subject`, `Enrollment`):

| Camada | Tipos | Responsabilidade |
|--------|-------|------------------|
| Packed Record | `StudentRecord`, `TeacherRecord`, `SubjectRecord`, `BTreeLeafValue` | Serialização binária direta (`#pragma pack(1)`, trivially copyable) |
| Model Wrapper | `Student`, `Teacher`, `Subject`, `Enrollment` | Getters/setters tipados, `serialize()`/`fromBytes()`, acesso a `.raw()` para bindings Lua |

Padrão de composição (não herança):

```cpp
class Student {
    StudentRecord raw_;  // composição do packed record
public:
    const StudentRecord& raw() const { return raw_; }
    std::vector<std::byte> serialize() const { return serializeRecord(raw_); }
    static Student fromBytes(const std::vector<std::byte>& bytes) {
        return Student(deserializeRecord<StudentRecord>(bytes));
    }
};
```

---

## 9. Testes Unitários

| # | Teste | Funcionalidade Validada |
|---|-------|------------------------|
| 1 | test_initialize | Inicialização do DataManager |
| 2 | test_create_student | Criação de aluno (7 campos + senha) |
| 3 | test_read_student | Leitura por ID via hash |
| 4 | test_read_nonexistent | Tratamento de ID inexistente |
| 5 | test_list_all_students | Listagem de alunos ativos |
| 6 | test_soft_delete | Soft delete com verificação de contagem |
| 7 | test_create_teacher_and_subject | CRUD professor + disciplina |
| 8 | test_enrollment | Matrícula via B+ Tree |
| 9 | test_update_grade | Atualização de nota (uint8_t) |
| 10 | test_enrollments_by_student | Consulta de matrículas por aluno |
| 11 | test_login | Autenticação CPF/senha (ambos papéis + falha) |

---

## 10. Instruções de Build

### 10.1 Pré-requisitos

#### Linux (Debian/Ubuntu)
```bash
sudo apt update
sudo apt install cmake gcc g++ libglfw3-dev ccache
```

#### macOS
```bash
brew update
brew install cmake glfw ccache
```

#### Windows (MSYS2/MinGW)
```powershell
pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc mingw-w64-x86_64-glfw mingw-w64-x86_64-ccache
```

### 10.2 Compilação

```bash
# Configurar
cmake -S . -B builds

# Compilar
cmake --build builds

# Executar GUI
./builds/compiled_executable/TPAEDSIII

# Executar testes
./builds/compiled_executable/run_tests
```

**Nota**: As dependências (hello_imgui, lua, luaaa) estão pré-clonadas em `libs/`. O build usa FetchContent com `SOURCE_DIR` apontando para os diretórios locais — sem download de internet. GLFW é dependência de sistema para OpenGL.

O CMake detecta automaticamente o gerador da sua plataforma (Unix Makefiles, Ninja, Visual Studio, Xcode). Para usar um gerador específico:

```bash
cmake -G Ninja -S . -B builds   # Mais rápido em todos os sistemas
cmake -G "Unix Makefiles" -S . -B builds
cmake -G "Visual Studio 17 2022" -S . -B builds  # Windows
cmake -G Xcode -S . -B builds   # macOS
```

### 10.3 Solução de Problemas

| Erro | Solução |
|------|---------|
| `Could not find GLFW` | `sudo apt install libglfw3-dev` (Linux) ou `brew install glfw` (macOS) |
| `scripts.bin` não encontrado | Executar a partir do diretório raiz do projeto ou `cmake --build builds` para regenerar |
| Erro de link com luaaa | Verificar se `libs/luaaa/` contém o repositório clonado |

---

## 11. Estrutura de Diretórios

```
TP/
├── src/
│   ├── main.cpp                        # Entry point + LuaStack specializations + bindings
│   ├── utility/                        # project_utility
│   │   ├── Constants.hpp               # Constantes (record sizes, chunk defaults, B+ tree params)
│   │   └── Enums.hpp                   # RecStatus, RecType, CrudOp, ViewId, ThemeColor
│   ├── model/                          # project_model
│   │   ├── Record.hpp                  # Packed structs: UserRecord, StudentRecord, TeacherRecord,
│   │   │                               # SubjectRecord, BTreeLeafValue, FileHeader, ChunkInfo
│   │   ├── Record.cpp                  # FileHeader/ChunkInfo serialization
│   │   ├── Student.hpp                 # Model wrapper (composição de StudentRecord)
│   │   ├── Teacher.hpp                 # Model wrapper (composição de TeacherRecord)
│   │   ├── Subject.hpp                 # Model wrapper (composição de SubjectRecord)
│   │   ├── Enrollment.hpp              # Model wrapper (composição de BTreeLeafValue)
│   │   ├── BPlusTree.hpp               # B+ Tree (PAGE_SIZE=4096, KEY_SIZE=20)
│   │   └── BPlusTree.cpp               # Insert/split, search/prefixRange, erase/merge
│   ├── controller/                     # project_controller
│   │   ├── DataManager.hpp             # Fachada CRUD exposta ao Lua + gerenciamento de chunks
│   │   ├── DataManager.cpp             # Implementação com chunks, B+ tree, índice hash
│   │   ├── FileManager.hpp             # I/O binário genérico (leitura/escrita/append)
│   │   ├── FileManager.cpp             # Read/write/append/markDeleted
│   │   ├── IndexCtrl.hpp               # Índice hash persistente (DJB2) + rebuild
│   │   └── IndexCtrl.cpp               # Insert/lookup/remove/nextId/rebuild
│   ├── view/                           # project_view
│   │   ├── bindings/ImguiBindings.cpp  # Bindings ImGui para Lua (Begin, Text, Button, etc.)
│   │   ├── ScriptArchive.hpp           # Empacotamento binário de scripts Lua
│   │   ├── ScriptArchive.cpp           # Load/save/registerAll de scripts.bin
│   │   └── scripts/                    # 15 scripts Lua (DSL)
│   │       ├── handlers/               # Navegação e autenticação
│   │       │   ├── router.lua          # Controlador de navegação + autenticação
│   │       │   ├── common.lua          # Sidebar, tema 4 cores, helpers UI
│   │       │   └── LoginView.lua       # Tela de login
│   │       ├── GUI/
│   │       │   ├── functional_views/   # Views CRUD (professor)
│   │       │   │   ├── MainMenu.lua
│   │       │   │   ├── StudentList.lua
│   │       │   │   ├── TeacherCreate.lua
│   │       │   │   ├── TeacherList.lua
│   │       │   │   ├── SubjectCreate.lua
│   │       │   │   └── SubjectList.lua
│   │       │   └── displayer_views/    # Views de exibição
│   │       │       ├── StudentCreate.lua
│   │       │       ├── StudentDetail.lua
│   │       │       ├── StudentProfile.lua
│   │       │       └── EnrollmentList.lua
│   │       └── misc/
│   │           ├── globals.lua
│   │           └── populate_samples.lua
├── libs/                               # Dependências pré-clonadas (FetchContent SOURCE_DIR)
│   ├── hello_imgui/                    # Hello ImGui v1.5.0 (inclui Dear ImGui)
│   ├── lua/                            # Lua 5.4.x
│   └── luaaa/                          # luaaa C++/Lua bindings
├── builds/                             # Diretório de build do CMake
│   ├── compiled_executable/            # Saída compilada (executáveis + assets)
│   │   ├── TPAEDSIII                   # GUI executável
│   │   ├── run_tests                   # Testes unitários
│   │   ├── scripts.bin                 # 15 scripts Lua empacotados
│   │   ├── pack_scripts                # Ferramenta de empacotamento
│   │   └── assets/                     # Fontes e assets copiados
│   └── ...                             # Artefatos internos do CMake
├── data/                               # Dados persistentes
│   ├── records.dat                     # Arquivo binário principal
│   └── test/                           # Diretório de testes (criado/removido dinamicamente)
├── assets/
│   └── fonts/                          # Fontes (DroidSans, FontAwesome)
├── tests/
│   └── test_main.cpp                   # 11 testes unitários
├── docs/
│   ├── Markdown/                       # Documentação em markdown
│   ├── Markdown/documentação_cmake.md  # Documentação detalhada do CMake
│   ├── UML Diagrams/                   # Diagramas PlantUML + PNG
│   ├── PDFs/                           # Documentos em PDF
│   └── ux/                             # Specs e mockups
├── tools/
│   └── pack_scripts.cpp                # Ferramenta CLI para empacotar scripts Lua
├── CMakeLists.txt
├── AGENTS.md
└── README.md
```

---

## 12. Diferenciais do Projeto

1. **Arquivo único com chunks**: 4 entidades no mesmo arquivo com layout determinístico
2. **B+ Tree como armazenamento de matrícula**: Sem EnrollmentRecord separado — a chave composta `ENR:STU:NNNN:SUB:MMMM` é o relacionamento
3. **Índice hash persistente**: Atualizado sincronamente, com rebuild periódico automático
4. **UserRecord com herança de layout**: StudentRecord e TeacherRecord compartilham campos comuns (name, email, cpf, password) via UserRecord
5. **Crescimento dinâmico**: Chunks duplicam quando cheios (amortized O(1))
6. **B+ Tree com 4096B páginas**: ~89 entries/leaf, prefix scan para consultas por aluno/disciplina
7. **IDs auto-incremento por tipo**: `padId()` com zero-padding para chaves ordenáveis
8. **Lua DSL**: 10 views modificáveis sem recompilação C++
9. **Autenticação integrada**: Login por CPF + senha com views condicionais por papel
10. **Build offline**: Dependências em `libs/`, sem FetchContent downloads
11. **Soft delete**: Status byte `'*'` mantém dados recuperáveis
12. **Grade inteira 0–100**: Sem ponto flutuante, sem ambiguidade de arredondamento
13. **Model wrappers com composição**: Separação clara entre packed structs e API pública
14. **ScriptArchive**: Scripts Lua empacotados em binário, sem dependência de sistema de arquivos em runtime

---

## 13. Conclusão

O projeto TP/AEDSIII implementa um sistema completo de matrículas acadêmicas com arquitetura MVC, persistência binária em chunks, índice hash persistente, Árvore B+ para relacionamentos N:N, autenticação por CPF/senha, e interface gráfica em Lua/ImGui com 10 views e tema minimalista de 4 cores. A separação entre packed records e model wrappers garante serialização segura sem sacrificar a expressividade da API pública. O sistema está funcional, com build verificado e 11 testes unitários, pronto para uso e extensão.

---

## Referências

- Hello ImGui: https://github.com/pthom/hello_imgui
- Lua: https://www.lua.org/docs.html
- Dear ImGui: https://github.com/ocornut/imgui
- luaaa: https://github.com/gengyong/luaaa
- PlantUML: https://plantuml.com

---

*Documento preparado para a disciplina AEDS III — PUC-MG*
*Para dúvidas técnicas, consulte `AGENTS.md`*

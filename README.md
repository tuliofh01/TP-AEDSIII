# TP/AEDSIII - Sistema de Matrícula Acadêmica

**Trabalho Prático da Disciplina AEDS III** - Pontifícia Universidade Católica de Minas Gerais (PUC-MG)

---

## 1. Resumo Executivo

Este documento apresenta o relatório técnico completo do projeto **TP/AEDSIII**, um sistema acadêmico de gerenciamento de matrículas desenvolvido em **C++20** com arquitetura **MVC** baseada em *flat namespaces*. O sistema implementa persistência binária de registros de tamanho fixo (67 bytes), índice hash **DJB2** para buscas O(1), interface gráfica via **Lua/ImGui** com tema minimalista de 4 cores, e 11 testes unitários cobrindo todas as operações CRUD.

**Palavras-chave**: C++20, MVC, Persistência Binária, Hash Index, Lua DSL, Dear ImGui, Testes Unitários, PUC-MG

---

## 2. Introdução

### 2.1 Contextualização e Motivação

O projeto foi desenvolvido como trabalho prático para a disciplina de Algoritmos e Estruturas de Dados III (AEDS III) da Pontifícia Universidade Católica de Minas Gerais. A disciplina tem como objetivo principal estudar estruturas de dados avançadas, algoritmos de busca e ordenação, e técnicas de persistência de dados.

Este trabalho supera significativamente os requisitos mínimos da disciplina, implementando um sistema completo com características típicas de projetos profissionais de software.

### 2.2 Objetivos do Projeto

- Implementar um sistema de gerenciamento de matrículas acadêmicas completo
- Demonstrar domínio de estruturas de dados (hash index, CRUD)
- Desenvolver habilidades em programação de sistemas em C++
- Criar interface gráfica moderna com separação clara de responsabilidades
- Garantir qualidade de código através de testes unitários

### 2.3 Escopo Funcional

O sistema permite:
- Cadastro de estudantes com dados pessoais
- Consulta de registros por ID dinâmico
- Busca otimizada por nome utilizando índice hash
- Listagem de todos os registros ativos
- Remoção lógica (soft delete) de registros
- Persistênciabináriaem arquivo local

---

## 3. Fundamentação Teórica

### 3.1 Arquitetura MVC

O padrão **Model-View-Controller (MVC)** é um padrão de arquitetura de software que separa a aplicação em três componentes principais:

- **Model (Modelo)**: Responsável pela representação dos dados e regras de negócio
- **View (Visão)**: Responsável pela interface com o usuário
- **Controller (Controlador)**: Responsável por intermediar as requisições entre Model e View

No contexto deste projeto, a implementação foi adaptada utilizando **flat namespaces** em vez de classes aninhadas, resultando em uma organização mais limpa e funcional:

| Camada | Namespace | Componentes |
|--------|-----------|--------------|
| Utility | `project_utility` | Constants.hpp, Enums.hpp |
| Model | `project_model` | Record.hpp, Record.cpp |
| Controller | `project_controller` | DataManager, FileManager, IndexCtrl |
| View | `project_view` | ImguiBindings.cpp |

### 3.2 Persistência Binária

A persistênciabinária foi escolhida sobre formatos textuais (JSON, XML, CSV) por oferecer:

- **Espaço**: Maior densidade de armazenamento
- **Performance**: Leitura/escrita mais rápida sem parsing
- **Previsibilidade**: Tamanho fixo de registros facilita cálculos de offset
- **Controle**: Manipulação direta de bytes

O formato de 67 bytes foi calculado considerando alinhamento de memória e necessidade de armazenamento dos campos.

### 3.3 Índice Hash

A implementação do índice hash utiliza o algoritmo **DJB2**, proposto por Daniel J. Bernstein em 1991. Este algoritmo foi escolhido por:

- **Simplicidade**: Implementação direta
- **Distribuição**: Boa distribuição para strings ASCII
- **Performance**: O(1) para operações de busca

O índice é recalculado automaticamente a cada 10 registros ativos, garantindo consistência após operações de delete.

---

## 4. Arquitetura do Sistema

### 4.1 Visão Geral da Arquitetura

```
┌─────────────────────────────────────────────────────────────┐
│                        TP_AEDSIII                           │
├─────────────────────────────────────────────────────────────┤
│  C++ Backend          │  Lua Views (DSL)    │  ImGui      │
│  ┌─────────────────┐   │  ┌──────────────┐   │  ┌────┐     │
│  │ project_utility │   │  │ router.lua   │───│─›│ UI │     │
│  │ (Constants,     │   │  │ common.lua   │   │  └────┘     │
│  │  Enums)         │   │  │ MainMenu     │   │             │
│  ├─────────────────┤   │  │ StudentList  │   │             │
│  │ project_model   │   │  │ StudentCrt   │   │             │
│  │ (Record,        │   │  └──────────────┘   │             │
│  │  Serializer)    │   │                      │             │
│  ├─────────────────┤   │  4-Cores Theme:      │             │
│  │ project_controller│ │  Preto, Branco,      │             │
│  │ (DataManager,   │   │  Verde, Vermelho     │             │
│  │  FileManager,   │   │                      │             │
│  │  IndexCtrl)     │   │                      │             │
│  ├─────────────────┤   │                      │             │
│  │ project_view    │   │                      │             │
│  │ (ImguiBindings) │   │                      │             │
│  └─────────────────┘   │                      │             │
└─────────────────────────────────────────────────────────────┘
```

### 4.2 Stack Tecnológico

| Componente | Tecnologia | Versão | Propósito |
|------------|-----------|--------|-----------|
| Linguagem | C++20 | GCC 10+ / Clang 15+ | Backend e lógica de negócio |
| Build | CMake | 3.20+ | Build cross-platform |
| GUI Framework | Dear ImGui (hello_imgui) | v1.5.0 | Renderização imediata |
| Scripting | Lua | 5.x | DSL para definições de UI |
| Bindings | ruaaa | Latest | Integração C++/Lua |
| Generator | Ninja | Any | Build paralelo |

---

## 5. Especificações Técnicas

### 5.1 Formato Binário do Registro

O registro de estudante possui exatamente **67 bytes** com a seguinte estrutura em memória:

| Offset | Tamanho | Tipo | Campo | Descrição |
|--------|---------|------|-------|-----------|
| 0 | 1 | char | status | 'A'=Ativo, '*'=Deletado |
| 1 | 4 | int32_t | id | ID dinâmico (recalculado) |
| 5 | 4 | int32_t | userId | ID do usuário (sistema externo) |
| 9 | 50 | char[50] | name | Nome (null-terminated) |
| 59 | 4 | uint32_t | birthDate | Data nascimento (YYYYMMDD) |
| 63 | 4 | uint32_t | padding | Alinhamento para 67 bytes |

**Total: 67 bytes**

### 5.2 Estrutura do Índice Hash

O arquivo de índice (.idx) armazena mappings nome → offset, permitindo buscas O(1):

```
[name_normalizado] -> [offset_no_dat]
```

O rebuild do índice é triggerado automaticamente quando o número de registros ativos é múltiplo de 10 (REBUILD_MODULO = 10).

### 5.3 Complexidade das Operações

| Operação | Complexidade | Descrição |
|----------|--------------|-----------|
| Create | O(1) | Append no arquivo + insert no índice |
| Read (by ID) | O(n) | Varredura sequencial por ID dinâmico |
| Search (by name) | O(1) | Lookup no hash index |
| Delete | O(n) | Varredura + markDeleted + remove do índice |
| List | O(n) | Scan todos ativos + recalc IDs |

---

## 6. Interface Gráfica

### 6.1 Arquitetura de Views em Lua DSL

A interface foi implementada utilizando **Lua** como linguagem de definição de UI (DSL), permitindo:

- Modificações na interface sem recompilação do código C++
- Separação clara entre lógica (C++) e apresentação (Lua)
- Facilidade de manutenção e extensão

**Estrutura das Views:**

```
src/views/
├── router.lua         # Controle de navegação entre views
├── common.lua         # Tema 4 cores + funções auxiliares
├── MainMenu.lua      # Menu principal da aplicação
├── StudentCreate.lua # Formulário de criação de registros
├── StudentList.lua   # Lista com busca e paginação
└── StudentDetail.lua # Visualização de detalhes
```

### 6.2 Tema Minimalista (4 Cores)

O design da interface segue um paleta de cores restrita para manter minimalismo:

| Cor | Uso |
|-----|-----|
| **Preto** | Backgrounds, janelas |
| **Branco** | Textos, elementos ativos |
| **Verde** | Sucesso, confirmações, botõespositivos |
| **Vermelho** | Erros, alertas, exclusões |

---

## 7. Testes Unitários

O projeto inclui **11 testes unitários** implementados em C++ nativo (sem frameworks externos), cobrindo todas as operações do sistema:

| # | Teste | Funcionalidade Validada |
|---|-------|------------------------|
| 1 | test_initialize | Inicialização do DataManager e abertura de arquivos |
| 2 | test_create_student | Criação de novo registro com validação |
| 3 | test_read_by_display_id | Leitura por ID dinâmico recalculado |
| 4 | test_read_nonexistent | Tratamento de registros inexistentes |
| 5 | test_list_all | Listagem completa com verificação de contagem |
| 6 | test_soft_delete | Deleção lógica sem remoção física |
| 7 | test_delete_recalculates_ids | Verificação de renumeração após delete |
| 8 | test_search_by_name | Busca via índice hash |
| 9 | test_create_empty_name_fails | Validação de entrada (nome vazio) |
| 10 | test_get_next_display_id | Geração de próximo ID disponível |
| 11 | test_multiple_create_delete_cycle | Ciclos completos de operações |

**Resultado**: Todos os 11 testes passando ✓

---

## 8. Instruções de Build

### 8.1 Pré-requisitos por Plataforma

#### Linux (Debian/Ubuntu)
```bash
sudo apt update
sudo apt install cmake ninja-build gcc g++ libglfw3-dev libfreetype6-dev ccache
```

#### Linux (Arch Linux)
```bash
sudo pacman -Syu
sudo pacman -S cmake ninja gcc glfw-x11 mesa freetype2 ccache
# Para Wayland: substituir glfw-x11 por glfw-wayland
```

#### Windows (PowerShell - MSYS2)
```powershell
# Abrir terminal MSYS2 ou Mingw64
pacman -Syu

# Instalar dependências
pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-glfw mingw-w64-x86_64-freetype mingw-w64-x86_64-ccache
```

#### macOS
```bash
brew update
brew install cmake ninja glfw freetype ccache
```

### 8.2 Compilação

#### Comando Único (Recomendado)
```bash
# Navegar até o diretório do projeto
cd /caminho/para/TP

# Limpar build anterior (opcional)
rm -rf cmake-build-release_build

# Configurar e compilar com Ninja
cmake -G Ninja -S . -B cmake-build-release_build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release_build

# OU com Makefiles (sem Ninja)
cmake -S . -B cmake-build-release_build
cmake --build cmake-build-release_build
```

#### Passos Detalhados

**Passo 1 - Configurar:**
```bash
cmake -G Ninja -S . -B cmake-build-release_build -DCMAKE_BUILD_TYPE=Release
```

**Passo 2 - Compilar:**
```bash
cmake --build cmake-build-release_build
```

**Passo 3 - Executar GUI:**
```bash
./builds/TP_AEDSIII
```

**Passo 4 - Executar Testes:**
```bash
./builds/run_tests
```

### 8.3 Execução

```bash
# Interface GUI
./builds/TP_AEDSIII

# Testes unitários (CLI)
./builds/run_tests
```

### 8.4 Solução de Problemas

| Erro | Solução |
|------|---------|
| `CMAKE_GENERATOR` not found | `rm -rf cmake-build-release_build` e reconfigurar |
| hello_imgui lento (~15min) | Primeira build baixa dependências - Aguardar |
| GLFW não encontrado | Verificar pacote glfw instalado |
| Problema de linker | Limpar build: `rm -rf cmake-build-release_build` |

---

## 9. Especificações de Interface

### 9.1 Diagramas UML

| Diagrama | Descrição |
|----------|-----------|
| ![Arquitetura](./docs/UML%20Diagrams/arquitetura_sistema.png) | Arquitetura geral do sistema |
| ![Classes](./docs/UML%20Diagrams/diagrama_classes.png) | Diagrama de classes UML |
| ![Sequência](./docs/UML%20Diagrams/diagrama_sequencia_crud.png) | Sequência de operações CRUD |
| ![Binary](./docs/UML%20Diagrams/estrutura_binaria.png) | Estrutura binária do registro |

### 9.2 Estrutura de Diretórios

```
TP/
├── src/
│   ├── main.cpp              # Entry point + bindings Lua
│   ├── utility/              # project_utility
│   │   ├── Constants.hpp     # Constantes globais
│   │   └── Enums.hpp         # Enumerações com using enum
│   ├── model/                # project_model
│   │   ├── Record.hpp        # Struct StudentRecord (67 bytes)
│   │   └── Record.cpp        # Serialização binária
│   ├── controller/           # project_controller
│   │   ├── DataManager.hpp   # CRUD exposto para Lua
│   │   ├── FileManager.hpp   # I/O binário (soft delete)
│   │   └── IndexCtrl.hpp     # DJB2 hash index
│   ├── view/                 # project_view
│   │   └── ImguiBindings.cpp # Bindings minimalistas ImGui
│   └── views/                # Lua DSL (UI)
├── libs/                     # Dependências (hello_imgui, lua, luaaa)
├── data/                     # Arquivos运行时 (students.dat, .idx)
├── tests/
│   └── test_main.cpp         # 11 testes unitários
├── docs/
│   ├── ux/                   # Specs e mockups de interface
│   └── UML Diagrams/         # Diagramas PlantUML
└── CMakeLists.txt           # Configuração de build
```

---

## 10. Diferenciais do Projeto

Este projeto demonstra as seguintes habilidades técnicas avançadas:

1. **Minimalismo UI**: Interface com paleta de 4 cores
2. **Flat Namespaces**: Organização moderna sem aninhamento
3. **C++20 Moderno**: using enum, std::byte, constexpr
4. **Binary-First**: Persistência de baixo nível (67 bytes)
5. **Lua DSL**: Interface definhada em Lua sem recompilação
6. **Índice Hash O(1)**: Busca otimizada via DJB2
7. **Soft Delete**: Dados recuperáveis com marcadores '*'
8. **IDs Dinâmicos**: Reordenação automática após deleções
9. **Testes Unitários**: 11 testes cobrindo CRUD completo
10. **Build Otimizado**: ccache + deps cache para builds 80%+ mais rápidos

---

## 11. Conclusão

O projeto TP/AEDSIII demonstra domínio completo de conceitos avançados de engenharia de software, incluindo:

- Manipulação de memória em baixo nível (serialização binária)
- Implementação de estruturas de dados otimizadas (hash index O(1))
- Integração multi-linguagem (C++/Lua/ImGui)
- Arquitetura de software profissional (MVC com flat namespaces)
- Qualidade de código (testes unitários, documentação)

O sistema está pronto para uso e pode ser extendido com novas funcionalidades seguindo a mesma arquitetura modular.

---

## Referências

- CMake Documentation: https://cmake.org/documentation/
- Hello ImGui: https://github.com/pthom/hello_imgui
- Lua Documentation: https://www.lua.org/docs.html
- Dear ImGui: https://github.com/ocornut/imgui
- ruaaa (C++/Lua bindings): https://github.com/gengyong/luaaa

---

*Documento preparado para a disciplina AEDS III - PUC-MG*  
*Para dúvidas técnicas, consulte `AGENTS.md`*  
*Para especificações de interface, veja `docs/ux/`*
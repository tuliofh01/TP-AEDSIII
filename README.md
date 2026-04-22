# TP/AEDSIII - Sistema de Matrícula Acadêmica

> Projeto de Trabalho Prático para a disciplina de AEDS III (PUC-MG)
> Arquitetura MVC com namespaces flat e GUI minimalista 4 cores

---

## 📋 Visão Geral

Este projeto é um sistema acadêmico de gerenciamento de matrículas com arquitetura moderna:
- **Backend**: C++20 com namespaces flat (project_utility, project_model, project_controller)
- **Frontend**: Interface gráfica via Lua/ImGui (DSL minimalista 4 cores)
- **Dados**: Arquivos binários de tamanho fixo (67 bytes/registro)
- **Índice**: Hash index para busca O(1) por nome

---

## 🏗️ Arquitetura do Sistema (Namespaces Flat - MVC)

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

---

## 🛠️ Stack Tecnológico

| Componente | Tecnologia | Propósito |
|------------|-----------|-----------|
| **Linguagem** | C++20 | Backend e lógica de negócio |
| **Build** | CMake 3.20+ (Ninja/Makefiles) | Build cross-platform |
| **GUI Framework** | Dear ImGui (via hello_imgui) | Renderização imediata |
| **Scripting** | Lua 5.x | DSL para definições de UI |
| **Bindings** | ruaaa | Integração C++/Lua |
| **Persistência** | Binary File I/O | Armazenamento customizado 67 bytes |
| **Índice** | DJB2 Hash | Busca O(1) por nome |

### Por que esta combinação?

1. **Minimalismo**: 4 cores (Preto, Branco, Verde, Vermelho)
2. **Performance**: C++ para manipulação direta de binário
3. **Flexibilidade**: Lua como DSL para UI
4. **Portabilidade**: CMake com dependências locais
5. **Flat Namespaces**: Sem aninhamento (project_utility, project_model, etc.)

---

## 📦 Estrutura de Diretórios (MVC Flat)

```
TP/
├── src/
│   ├── main.cpp              # Entry point (Lua + ImGui)
│   ├── utility/              # project_utility
│   │   ├── Constants.hpp     # NAME_LEN, OFFSET_*, REBUILD_MODULO
│   │   └── Enums.hpp         # RecStatus, ViewId (using enum)
│   ├── model/                # project_model
│   │   ├── Record.hpp        # StudentRecord (67 bytes)
│   │   └── Record.cpp        # Serialização binária
│   ├── controller/           # project_controller
│   │   ├── DataManager.hpp   # CRUD exposto para Lua
│   │   ├── DataManager.cpp
│   │   ├── FileManager.hpp  # I/O binário (soft delete)
│   │   ├── FileManager.cpp
│   │   ├── IndexCtrl.hpp     # DJB2 hash index
│   │   └── IndexCtrl.cpp
│   ├── view/                 # project_view
│   │   └── ImguiBindings.cpp # Minimal ImGui bindings
│   └── views/                # Lua UI (DSL)
│       ├── router.lua        # Navegação
│       ├── common.lua        # 4-cores theme
│       ├── MainMenu.lua
│       ├── StudentCreate.lua
│       ├── StudentList.lua
│       └── StudentDetail.lua
├── libs/                     # Dependências fetchadas
├── data/                     # students.dat, students.idx
├── tests/
│   └── test_main.cpp         # 11 testes unitários
├── docs/
│   ├── ux/                   # Mockups e specs de UI
│   │   ├── specs/interface_spec.md
│   │   └── mockups/interface_mockups.md
│   └── UML Diagrams/         # PlantUML diagrams
├── CMakeLists.txt
└── README.md
```

---

## 🧪 Recursos

### Backend (C++)

- **Persistência Binária**: Registros de tamanho fixo (67 bytes)
- **CRUD Completo**: Create, Read, Update, Delete (soft delete com '*')
- **IDs Dinâmicos**: Recalculados a cada listagem
- **Índice Hash**: DJB2 para busca O(1) por nome normalizado
- **Rebuild Trigger**: A cada 10 registros ativos
- **Flat Namespaces**: project_utility, project_model, project_controller, project_view

### Frontend (Lua - 4 Cores)

- **Router**: Navegação entre views
- **Theme**: 4 cores (Preto, Branco, Verde, Vermelho)
- **Views**: MainMenu, StudentCreate, StudentList, StudentDetail
- **Minimalismo**: Sem componentes desnecessários

---

## 🛠️ Instruções de Build

### Pré-requisitos

| Plataforma | Requisitos |
|------------|-----------|
| **Linux** | GCC 10+, CMake 3.20+, Ninja, OpenGL, GLFW |
| **macOS** | Clang 15+, CMake 3.20+, Ninja, OpenGL |
| **Windows** | MinGW-w64 (GCC 10+), CMake 3.20+, Ninja, OpenGL, GLFW |

### Compilação

```bash
# Configurar (Ninja - mais rápido)
cmake -G Ninja -S . -B cmake-build-release_build

# Compilar
cmake --build cmake-build-release_build

# OU com Makefiles (cross-platform padrão)
cmake -S . -B cmake-build-release_build
cmake --build cmake-build-release_build
```

### Execução

```bash
# GUI
./builds/TP_AEDSIII

# Testes CLI
./builds/run_tests
```

---

## 🚀 Otimizando o Processo de Build

Guia para acelerar compilações e resolver erros comuns, com foco em Linux e Windows.

### Pré-requisitos

#### Linux (Debian/Ubuntu/Arch)
```bash
# Debian/Ubuntu
sudo apt install cmake ninja-build gcc g++ libglfw3-dev libfreetype6-dev ccache

# Arch Linux
sudo pacman -S cmake ninja gcc glfw-x11 mesa freetype2 ccache
# Para Wayland: substitua glfw-x11 por glfw-wayland
```

#### Windows (MSYS2)
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-glfw mingw-w64-x86_64-freetype ccache
```

#### macOS
```bash
brew install cmake ninja glfw freetype ccache
```

### Recursos de Aceleração

O projeto já inclui otimizações em `CMakeLists.txt`:

1. **Cache de dependências**: `.deps_cache/` evita re-downloads de hello_imgui, lua e luaaa
2. **ccache**: Reduz tempo de rebuild em 80%+
3. **hello_imgui enxuto**: Recursos não usados (SDL2, PlutoSVG, Vulkan) desativados
4. **Builds incrementais**: Evite `rm -rf` do diretório de build; use `cmake --build` para rebuilds

### Desafios Comuns

| Erro | Causa | Solução |
|------|-------|---------|
| Conflito de gerador CMake | Cache de build antigo (ex: Unix Makefiles) | `rm -rf cmake-build-release_build` |
| hello_imgui lento (~10-15min) | Primeira build baixa dependências grandes | Cache em `.deps_cache` elimina tempo em builds futuros |
| Falha no clone de dependências | Branch `master` instável | Já fixado: hello_imgui v1.5.0 |
| Dependências faltantes | GLFW3/Freetype não instalados | Instale pacotes de pré-requisitos |

### Comando de Build Rápido

```bash
# Linux/macOS
rm -rf cmake-build-release_build
cmake -G Ninja -S . -B cmake-build-release_build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release_build
```

---

## 📚 Documentação

| Arquivo | Conteúdo |
|---------|----------|
| `README.md` | Visão geral e build |
| `AGENTS.md` | Guia técnico detalhado |
| `docs/ux/specs/interface_spec.md` | Especificação UI 4 cores |
| `docs/ux/mockups/interface_mockups.md` | Mockups ASCII |
| `docs/UML Diagrams/*.puml` | Diagramas PlantUML |

---

## 🔧 Recursos do Sistema

### Formato Binário (67 bytes/registro)

| Offset | Tamanho | Campo |
|--------|---------|-------|
| 0 | 1 byte | Status ('A' = Ativo, '*' = Deletado) |
| 1 | 4 bytes | ID (int32_t) |
| 5 | 4 bytes | UserID (int32_t) |
| 9 | 50 bytes | Nome (char[50]) |
| 59 | 4 bytes | DataNascimento (uint32_t) |

### Índice Hash

- **Algoritmo**: DJB2 (hash = hash * 33 + char)
- **Normalização**: Lowercase
- **Rebuild**: A cada 10 registros ativos (REBUILD_MODULO = 10)

### Namespace Flat (C++20)

```cpp
namespace project_utility {
    enum class RecStatus : char { Ativo = 'A', Deletado = '*' };
    using enum RecStatus;  // C++20: Acesso direto
    
    inline constexpr size_t NAME_LEN = 50;
    inline constexpr size_t RECORD_TOTAL_SIZE = 67;
}
```

---

## 🎯 Diferenciais do Projeto

1. **Minimalismo UI**: 4 cores (Preto, Branco, Verde, Vermelho)
2. **Flat Namespaces**: Sem aninhamento (project_*)
3. **C++20 Moderno**: using enum, std::byte, std::to_integer
4. **Binary-First**: 67 bytes fixos = previsibilidade total
5. **Lua DSL**: Views em Lua sem recompilar C++

---

*Para dúvidas técnicas, consulte `AGENTS.md`.*
*Para arquitetura GUI, veja `docs/ux/`.*
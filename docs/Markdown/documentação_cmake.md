# Documentação do Sistema de Build CMake — Sistema de Matrícula Acadêmica (AEDS III)

## Sumário

1. [Introdução ao CMake](#1-introdução-ao-cmake)
2. [Por que o CMake é essencial para projetos C++ modernos](#2-por-que-o-cmake-é-essencial-para-projetos-c-modernos)
3. [Análise Estrutural do CMakeLists.txt](#3-análise-estrutural-do-cmakeliststxt)
4. [Conceitos Chave do CMake](#4-conceitos-chave-do-cmake)
5. [Explicação do Processo de Build](#5-explicação-do-processo-de-build)
6. [Apêndice: Referência de Comandos CMake](#6-apêndice-referência-de-comandos-cmake)

---

## 1. Introdução ao CMake

CMake (Cross-Platform Make) é um sistema de build de código aberto e
multi-plataforma. Ele gerencia o processo de build de maneira
independente do compilador, produzindo arquivos de build nativos
(Makefiles, arquivos Ninja, projetos Visual Studio, etc.) para diferentes
plataformas.

**Características Principais:**
- Multi-plataforma: Funciona em Linux, macOS, Windows
- Independente de linguagem: Principalmente usado para C/C++ mas suporta outras
- Baseado em geradores: Cria arquivos de build para múltiplos sistemas
- Modular: Suporta `find_package`, `FetchContent` e módulos personalizados

---

## 2. Por que o CMake é essencial para projetos C++ modernos

### 2.1 Gerenciamento Complexo de Dependências

Projetos C++ modernos frequentemente dependem de múltiplas bibliotecas
externas. O CMake simplifica isso através de:

```
Seu Projeto → Build CMake → Executável
                    │
         ┌──────────┼──────────┐
    hello_imgui    Lua      luaaa
```

### 2.2 Compatibilidade Multi-plataforma

O mesmo `CMakeLists.txt` funciona em diferentes plataformas:

| Plataforma | Geradores Possíveis | Compilador |
|------------|--------------------|------------|
| Linux      | Ninja, Unix Makefiles | GCC, Clang |
| macOS      | Ninja, Xcode       | Clang       |
| Windows    | Ninja, Visual Studio | MSVC, MinGW |

### 2.3 Busca de Dependências (FetchContent)

CMake pode automaticamente baixar e compilar dependências:

```cmake
FetchContent_Declare(hello_imgui
    GIT_REPOSITORY https://github.com/pthom/hello_imgui.git
    GIT_TAG        v1.5.0
    SOURCE_DIR    "${LOCAL_LIBS_DIR}/hello_imgui"
)
FetchContent_MakeAvailable(hello_imgui)
```

Isso elimina a necessidade de instalação manual de bibliotecas. Neste projeto, as dependências estão pré-clonadas em `libs/` para builds offline.

---

## 3. Análise Estrutural do CMakeLists.txt

### 3.1 Cabeçalho e Versão Mínima

```cmake
cmake_minimum_required(VERSION 3.20)
project(TPAEDSIII LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

- `cmake_minimum_required(VERSION 3.20)`: Versão mínima do CMake
- `project(TPAEDSIII LANGUAGES CXX C)`: Nome do projeto e linguagens (C++ e C)
- `set(CMAKE_CXX_STANDARD 20)`: Padrão C++20
- `set(CMAKE_CXX_STANDARD_REQUIRED ON)`: C++20 obrigatório

### 3.2 Build Speed Optimizations

```cmake
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()
```

Usa ccache para acelerar recompilações. As compilações são armazenadas em cache e reutilizadas quando os arquivos fonte não mudam.

### 3.3 FetchContent — Gerenciamento de Dependências

```cmake
include(FetchContent)
set(LOCAL_LIBS_DIR "${CMAKE_SOURCE_DIR}/libs")
```

| Biblioteca | Propósito |
|------------|-----------|
| hello_imgui | Framework GUI com integração ImGui |
| lua | Núcleo da linguagem de script Lua |
| luaaa | Biblioteca de ligação C++ para Lua |

As três dependências são declaradas com `FetchContent_Declare()` apontando `SOURCE_DIR` para os diretórios locais em `libs/`. Isso permite builds offline — sem download de internet.

### 3.4 Backend OpenGL

```cmake
find_package(OpenGL REQUIRED)
set(PLATFORM_LIBS OpenGL::GL)
```

Usa OpenGL para renderização gráfica em todas as plataformas.

### 3.5 Funções CMake do hello_imgui

```cmake
set(HELLO_IMGUI_CMAKE_PATH "${CMAKE_SOURCE_DIR}/libs/hello_imgui/hello_imgui_cmake")
include(${HELLO_IMGUI_CMAKE_PATH}/hello_imgui_add_app.cmake)
include(${HELLO_IMGUI_CMAKE_PATH}/hello_imgui_build_lib.cmake)
```

Inclui funções CMake personalizadas da biblioteca hello_imgui que simplificam a criação de aplicações baseadas em ImGui.

### 3.6 Build Output Directory

```cmake
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/builds/compiled_executable)
```

Define o diretório de saída para os executáveis compilados em `builds/compiled_executable/`.

Assets (fontes) são copiados para o diretório de saída em tempo de configuração:

```cmake
if(EXISTS ${CMAKE_SOURCE_DIR}/assets)
    file(COPY ${CMAKE_SOURCE_DIR}/assets/ DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets)
endif()
```

### 3.7 Script Packing Tool

```cmake
add_executable(pack_scripts
    ${CMAKE_SOURCE_DIR}/tools/pack_scripts.cpp
    ${CMAKE_SOURCE_DIR}/src/view/ScriptArchive.cpp
)
```

Ferramenta CLI que empacota todos os scripts Lua em um único arquivo `scripts.bin`.

### 3.8 Fontes e Alvos

Os fontes C++ são listados explicitamente (sem `GLOB_RECURSE`):

```cmake
set(APP_SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/view/ScriptArchive.cpp
    ${CMAKE_SOURCE_DIR}/src/view/bindings/ImguiBindings.cpp
    ${CMAKE_SOURCE_DIR}/src/model/Record.cpp
    ${CMAKE_SOURCE_DIR}/src/model/BPlusTree.cpp
    ${CMAKE_SOURCE_DIR}/src/controller/FileManager.cpp
    ${CMAKE_SOURCE_DIR}/src/controller/IndexCtrl.cpp
    ${CMAKE_SOURCE_DIR}/src/controller/DataManager.cpp
)
```

**Alvos:**
- `TPAEDSIII`: Aplicação GUI principal
- `run_tests`: Testes unitários (CLI)
- `pack_scripts`: Ferramenta de empacotamento

### 3.9 Rebuild Automático de scripts.bin

```cmake
add_custom_target(pack_scripts_always ALL
    COMMAND $<TARGET_FILE:pack_scripts>
        ${CMAKE_SOURCE_DIR}/src/view/scripts
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts.bin
    DEPENDS pack_scripts
)
```

Executa o `pack_scripts` em todo build, garantindo que `scripts.bin` reflita o estado atual da pasta `src/view/scripts/`.

### 3.10 Compiler Flags

```cmake
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /permissive-)
endif()
```

Flags de aviso específicas do compilador para garantir conformidade com padrões.

---

## 4. Conceitos Chave do CMake

### 4.1 Alvos (Targets)

Um alvo representa um artefato de build (executável ou biblioteca).

| Tipo de Alvo | Exemplos |
|-------------|----------|
| executável  | `TPAEDSIII`, `run_tests`, `pack_scripts` |
| biblioteca obj | `liblua` (Lua) |
| biblioteca linkada | `hello-imgui::hello_imgui`, `OpenGL::GL` |

### 4.2 Propriedades

Alvos têm propriedades que configuram como são construídos:
- `SOURCES`: Arquivos de fonte para compilar
- `INCLUDE_DIRECTORIES`: Caminhos de busca de cabeçalho
- `COMPILE_OPTIONS`: Flags do compilador
- `LINK_LIBRARIES`: Bibliotecas para linkar

### 4.3 Especificadores de Acesso

| Specifier | Descrição |
|-----------|-----------|
| `PUBLIC`  | Propaga para dependentes |
| `PRIVATE` | Usado apenas por este alvo |
| `INTERFACE` | Apenas necessário por dependentes |

### 4.4 Variáveis

**Definidas pelo projeto:**
- `APP_SOURCES`: Arquivos de fonte para aplicação principal
- `LOCAL_LIBS_DIR`: Localização das bibliotecas baixadas
- `PLATFORM_LIBS`: Bibliotecas específicas da plataforma

**Definidas pelo CMake:**
- `CMAKE_SOURCE_DIR`: Diretório raiz do projeto
- `CMAKE_BINARY_DIR`: Diretório de build
- `CMAKE_CXX_STANDARD`: Versão do padrão C++

---

## 5. Explicação do Processo de Build

### 5.1 Fase de Configuração

```bash
cmake -S . -B builds
```

Passo a passo:
1. CMake lê o `CMakeLists.txt`
2. Processa `FetchContent` (localiza dependências em `libs/`)
3. Gera regras de build (Makefiles, Ninja, ou IDE)
4. Saída para `builds/`

### 5.2 Fase de Build

```bash
cmake --build builds
```

Passo a passo:
1. O gerador lê os arquivos de build gerados
2. Compila arquivos de fonte (`.cpp` → `.o`)
3. Linka arquivos objeto com bibliotecas
4. Produz executável em `builds/compiled_executable/`

### 5.3 Execução

```bash
./builds/compiled_executable/TPAEDSIII
```

O executável:
1. Inicializa estado Lua
2. Carrega ligações ImGui personalizadas
3. Cria `DataManager` exposto ao Lua
4. Executa `HelloImGui` com UI definida em Lua

---

## 6. Apêndice: Referência de Comandos CMake

| Comando | Propósito |
|---------|-----------|
| `cmake_minimum_required()` | Define versão mínima do CMake |
| `project()` | Define nome do projeto e linguagens |
| `set()` | Define valor de variável |
| `add_executable()` | Cria alvo executável |
| `add_library()` | Cria alvo biblioteca |
| `target_include_directories()` | Adiciona caminhos de busca de cabeçalho |
| `target_link_libraries()` | Liga bibliotecas ao alvo |
| `find_package()` | Encontra pacote externo |
| `include()` | Inclui módulo CMake |
| `file()` | Operações com arquivos |
| `if()/elseif()/else()/endif()` | Lógica condicional |
| `foreach()` | Iteração de loop |
| `message()` | Imprime mensagens |
| `FetchContent_Declare()` | Declara dependência externa |
| `FetchContent_MakeAvailable()` | Baixa e configura dependência |

---

*Versão do Documento: 1.1*
*Última Atualização: 2026-06-14*
*Projeto: TPAEDSIII — Sistema de Matrícula Acadêmica*
*Disciplina: AEDS III — PUC-MG*

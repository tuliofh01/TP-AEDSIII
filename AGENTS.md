# AGENTS.md - TP/AEDSIII Build and Development Guide

> Academic Enrollment System (AEDS III) - Technical Documentation
> Last Updated: 2026-04-27

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Architecture](#2-architecture)
3. [Build Instructions](#3-build-instructions)
4. [Running the Application](#4-running-the-application)
5. [Running Tests](#5-running-tests)
6. [Project Structure](#6-project-structure)
7. [Code Conventions](#7-code-conventions)
8. [Extending the System](#8-extending-the-system)
9. [Build Logs](#9-build-logs)

---

## 1. Project Overview

**TP (Trabalho Prático)** is an academic enrollment system built for the AEDS III course at PUC-MG (Pontifícia Universidade Católica de Minas Gerais).

### Core Technologies

| Technology | Purpose |
|------------|---------|
| C++20 | Core language |
| CMake 3.20+ | Build system |
| Lua 5.x | GUI scripting (DSL) |
| Dear ImGui | GUI framework via hello_imgui |
| Binary File I/O | Custom data persistence |

### Key Features

- Binary file persistence with fixed-size records (66 bytes per Student)
- CRUD operations via DAO pattern
- Lua-based GUI views (acts as DSL)
- Cross-platform support (Windows, macOS, Linux)

---

## 2. Architecture

```
src/
├── main.cpp              # Entry point (Lua + Dear ImGui)
├── models/
│   └── Student.h       # Entity (66 bytes binary)
├── dao/
│   ├── StudentDAO.h    # Data Access Object
│   └── StudentDAO.cpp  # CRUD implementation
├── ctrls/
│   ├── DataManager.hpp # Controller class
│   └── DataManager.cpp
├── utils/
│   └── FileManager.h  # File utilities
└── views/
    ├── router.lua     # Navigation controller
    ├── MainMenu.lua  # Main menu view
    ├── StudentView.lua
    ├── CourseView.lua
    └── common.lua   # Shared UI components

libs/                    # External dependencies (FetchContent)
├── hello_imgui/
├── lua/
├── ruaaa/
└── imgui_lua_bindings/

data/
└── students.dat       # Binary data file

tests/
└── test_main.cpp   # Unit tests (CLI)
```

### Data Flow

```
C++ main.cpp 
  → Initialize Lua State
  → Load ImGui bindings
  → Bind DataManager to Lua
  → Load router.lua
  → HelloImGui::Run() 
    → RenderUI() callback (Lua)
      → router.lua
        → View modules (MainMenu, StudentView, etc.)
```

---

## 3. Build Instructions

### Prerequisites

| Platform | Requirements |
|----------|--------------|
| Linux | GCC 10+, CMake 3.20+, Ninja, OpenGL, GLFW |
| macOS | Clang 15+, CMake 3.20+, Ninja, OpenGL |
| Windows | MinGW-w64 (GCC 10+), CMake 3.20+, Ninja, OpenGL, GLFW |

### 3.1 Linux

```bash
# Clean previous build (if needed)
rm -rf cmake-build-release_build/CMakeCache.txt cmake-build-release_build/CMakeFiles

# Configure with Ninja
cmake -G Ninja -S . -B cmake-build-release_build

# Build
cmake --build cmake-build-release_build
```

### 3.2 macOS

```bash
# Clean previous build
rm -rf cmake-build-release_build/CMakeCache.txt cmake-build-release_build/CMakeFiles

# Configure (Ninja or Makefiles)
cmake -G Ninja -S . -B cmake-build-release_build

# Build
cmake --build cmake-build-release_build
```

### 3.3 Windows (MinGW)

```powershell
# Clean previous build
Remove-Item -Recurse -Force cmake-build-release_build/CMakeCache.txt, cmake-build-release_build/CMakeFiles

# Configure with Ninja and MinGW
cmake -G Ninja -S . -B cmake-build-release_build

# Build
cmake --build cmake-build-release_build
```

### Alternative: Using Makefiles (if Ninja not available)

```bash
cmake -S . -B cmake-build-release_build
cmake --build cmake-build-release_build
```

---

## 4. Running the Application

### GUI Mode

After successful build:

```bash
cd builds
./TP_AEDSIII
```

Expected behavior:
- Opens 800x600 window titled "AEDS III - Control Panel"
- Navigation sidebar on left
- Main content area on right

### Command Line Arguments

None currently supported (v1.0).

---

## 5. Running Tests

### Build Tests Only

```bash
cmake --build cmake-build-release_build --target run_tests
```

### Run Tests

```bash
./builds/run_tests
```

Expected output:
```
test_create_and_read passed!
test_remove passed!

All tests passed successfully!
```

---

## 6. Project Structure

### Source Files

| Path | Description |
|------|------------|
| `src/main.cpp` | Application entry point |
| `src/models/Student.h` | Student entity (66 bytes) |
| `src/dao/StudentDAO.h` | Student DAO interface |
| `src/dao/StudentDAO.cpp` | Student DAO implementation |
| `src/ctrls/DataManager.hpp` | Controller header |
| `src/ctrls/DataManager.cpp` | Controller implementation |
| `src/utils/FileManager.h` | File utilities |
| `src/views/router.lua` | View router/navigation |
| `src/views/MainMenu.lua` | Main menu view |
| `src/views/StudentView.lua` | Student management view |
| `src/views/CourseView.lua` | Course management view |
| `src/views/common.lua` | Shared UI components |

### Data Files

| Path | Description |
|------|-------------|
| `data/students.dat` | Binary student records |
| `builds/TP_AEDSIII` | GUI executable |
| `builds/run_tests` | Test executable |

### Library Files (Fetched)

| Path | Source |
|------|--------|
| `libs/hello_imgui/` | https://github.com/pthom/hello_imgui.git |
| `libs/lua/` | https://github.com/marovira/lua.git |
| `libs/luaaa/` | https://github.com/gengyong/luaaa.git |
| `libs/imgui_lua_bindings/` | https://github.com/patrickriordan/imgui_lua_bindings.git |

---

## 7. Code Conventions

### C++ Style

- **Standard**: C++20 (`set(CMAKE_CXX_STANDARD 20)`)
- **Include Guards**: `#ifndef FILE_H` / `#define FILE_H` / `#endif`
- **Naming**: 
  - Classes: `PascalCase` (e.g., `StudentDAO`)
  - Methods: `camelCase` (e.g., `processItem`)
  - Members: `_camelCase` (e.g., `_status`)
- **Memory**: Use `std::unique_ptr` for ownership transfer
- **Integer Types**: Use `<cstdint>` (`std::int32_t`, `std::uint32_t`)

### Lua Style

- **Modules**: Return table with exports
- **Functions**: `camelCase`
- **Constants**: `UPPER_SNAKE_CASE`

```lua
local M = {}

function M.render()
    -- Implementation
end

return M
```

### Binary Persistence Rules

- Fixed-size records only (no pointers)
- Use `<cstdint>` types for portability
- No `std::string` in persistable structs
- Logical deletion via `bool removed` flag

---

## 8. Extending the System

### 8.1 Adding a New Entity

1. Create model in `src/models/NewEntity.h`
2. Create DAO in `src/dao/NewEntityDAO.h` and `.cpp`
3. Add to CMakeLists.txt if needed

Example:
```cpp
// src/models/Course.h
#pragma once
#include <cstdint>

struct Course {
    bool removed;
    std::int32_t _id;
    char name[50];
    char code[20];
    std::int32_t credits;
};
```

### 8.2 Adding a New View

1. Create Lua file in `src/views/NewView.lua`
2. Module must export `render()` function
3. Register in `router.lua`:

```lua
local NewView = require("src.views.NewView")

-- In router.lua renderContent():
elseif currentView == "new" then
    NewView.render()
end
```

### 8.3 Binding C++ to Lua

Use ruaaa in `main.cpp`:

```cpp
#include "../libs/luaaa/luaaa.hpp"

luaaa::LuaClass<MyClass> luaMyClass(L, "MyClass");
luaMyClass.ctor();
luaMyClass.fun("myMethod", &MyClass::myMethod);
luaMyClass.get("myProperty", &MyClass::getProperty);
```

---

## 9. Build Logs

### Build #2 - 2026-04-29

**Status**: ✅ Completed

**Changes Made**:
1. Fixed include paths in main.cpp to use CMake-managed include directories instead of hardcoded relative paths
2. Fixed LoadImguiBindings function signature to match actual implementation
3. Added hello_imgui include path to CMakeLists.txt
4. Replaced incompatible imgui_lua_bindings with custom minimal ImGui bindings (src/lua/ImguiBindings.cpp)
5. Fixed CMake target name: `hello_imgui::hello_imgui` → `hello-imgui::hello_imgui`

**Issues Resolved**:
- imgui_lua_bindings was incompatible with newer ImGui API (built with hello_imgui)
- Created custom minimal binding with essential functions: Begin, End, Text, Button, Separator, SetNextWindowPos, SetNextWindowSize, InputText, InputInt, InputFloat, SameLine, etc.

**Build Command**:
```bash
rm -rf cmake-build-release_build
cmake -G Ninja -S . -B cmake-build-release_build
cmake --build cmake-build-release_build
```

### Build #1 - 2026-04-27

**Status**: ✅ Completed

**Changes Made**:
1. Fixed CMake generator conflict (removed Unix Makefiles cache)
2. Rewrote CMakeLists.txt with cross-platform support (Ninja + Windows/Mac/Linux)
3. Fixed DataManager declaration and implementation
4. Created router.lua with view navigation
5. Updated main.cpp with Lua path setup
6. Created AGENTS.md

**Build Command**:
```bash
cmake -G Ninja -S . -B cmake-build-release_build
cmake --build cmake-build-release_build
```

---

## Troubleshooting

### CMake: Generator Mismatch

**Error**: `Does not match the generator used previously`

**Fix**: Clear CMake cache
```bash
rm -rf cmake-build-release_build/CMakeCache.txt cmake-build-release_build/CMakeFiles
```

### CMake: Missing Dependencies

**Error**: `Could not find GLFW`

**Fix**: Install system dependencies
- Linux: `sudo apt install libglfw3-dev libopengl-dev`
- macOS: `brew install glfw`
- Windows: MinGW includes GLFW (via MSYS2)

### Lua: File Not Found

**Error**: `Lua Script Error: cannot open src/views/router.lua`

**Fix**: Run from project root directory, or check working directory in IDE.

---

## Quick Reference

| Command | Action |
|---------|--------|
| `cmake -G Ninja -S . -B build` | Configure |
| `cmake --build build` | Build |
| `./builds/TP_AEDSIII` | Run GUI |
| `./builds/run_tests` | Run Tests |
| `rm -rf build/CMakeCache.txt build/CMakeFiles` | Clean Cache |

---

*This document is maintained with the project. Last updated: 2026-04-27*
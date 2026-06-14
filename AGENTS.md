# AGENTS.md - TP/AEDSIII Build and Development Guide

> Academic Enrollment System (AEDS III) - Technical Documentation
> Last Updated: 2026-06-14

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
9. [Troubleshooting](#9-troubleshooting)

---

## 1. Project Overview

An academic enrollment system built for the AEDS III course at PUC-MG.

### Core Technologies

| Technology | Purpose |
|------------|---------|
| C++20 | Core language |
| CMake 3.20+ | Build system |
| Lua 5.x | GUI scripting (DSL) |
| Dear ImGui | GUI framework via hello_imgui |
| Binary File I/O | Custom data persistence with B+ Tree indexing |

### Key Features

- Binary file persistence with packed fixed-size records (StudentRecord 141 B, TeacherRecord 167 B, SubjectRecord 84 B)
- B+ Tree indexed storage for enrollment records
- Lua-based GUI views (acts as DSL)
- CPF+password login with teacher/student role support
- Cross-platform support (Windows, macOS, Linux)

---

## 2. Architecture

```
src/
├── main.cpp                    # Entry point (Lua + Dear ImGui)
├── model/
│   ├── Record.hpp              # Packed binary records + generic memcpy serialization
│   ├── Record.cpp              # Serialization helpers
│   ├── BPlusTree.hpp           # B+ Tree indexed storage
│   ├── BPlusTree.cpp           # B+ Tree implementation
│   ├── Student.hpp             # Student model wrapper
│   ├── Teacher.hpp             # Teacher model wrapper
│   ├── Subject.hpp             # Subject model wrapper
│   └── Enrollment.hpp          # Enrollment model wrapper
├── controller/
│   ├── DataManager.hpp         # Main controller (CRUD + login)
│   ├── DataManager.cpp
│   ├── FileManager.hpp         # Low-level binary file I/O
│   ├── FileManager.cpp
│   ├── IndexCtrl.hpp           # In-memory hash-map index
│   └── IndexCtrl.cpp
├── view/
│   ├── ScriptArchive.hpp       # Binary archive of Lua scripts
│   ├── ScriptArchive.cpp
│   ├── bindings/
│   │   └── ImguiBindings.cpp   # Custom minimal ImGui -> Lua bindings
│   └── scripts/
│       ├── handlers/
│       │   ├── router.lua      # View router/navigation
│       │   └── common.lua      # Shared UI helpers, navigation
│       ├── GUI/
│       │   ├── displayer_views/
│       │   │   ├── EnrollmentList.lua
│       │   │   ├── StudentCreate.lua
│       │   │   ├── StudentDetail.lua
│       │   │   └── StudentProfile.lua
│       │   └── functional_views/
│       │       ├── LoginView.lua     # Login screen
│       │       ├── MainMenu.lua
│       │       ├── StudentList.lua
│       │       ├── SubjectCreate.lua
│       │       ├── SubjectList.lua
│       │       ├── TeacherCreate.lua
│       │       └── TeacherList.lua
│       └── misc/
│           ├── globals.lua         # Global Lua state
│           └── populate_samples.lua# Sample data generation (auto-populate)
├── utility/
│   ├── Constants.hpp           # Compile-time constants
│   └── Enums.hpp               # Enumerations

builds/compiled_executable/          # Build output (executable + scripts.bin + assets)
data/                         # Runtime binary data files (records.dat, records.idx)
assets/fonts/                 # Fonts for ImGui
builds/                       # CMake build tree (internals, not output)
libs/                         # External dependencies
├── hello_imgui/
├── lua/
└── luaaa/
tools/
└── pack_scripts.cpp          # Script packing tool
tests/
└── test_main.cpp             # 11 unit tests (CLI)
```

### Data Flow

```
C++ main.cpp
  -> Initialize Lua State
  -> Load ImGui bindings (custom ImguiBindings.cpp)
  -> Register DataManager methods via LuaModule + light userdata
  -> Load scripts.bin binary archive via ScriptArchive
  -> HelloImGui::Run()
    -> RenderUI() callback (Lua)
      -> router.lua
        -> View modules (LoginView, MainMenu, StudentList, etc.)
```

### Binary File Layout

Single `records.dat` with `records.idx` hash index:

```
[FileHeader 256 B][ChunkTable 4x32 B][Chunk S][Chunk T][Chunk B][Chunk I]
```

- Chunk S: StudentRecord (141 B each)
- Chunk T: TeacherRecord (167 B each)
- Chunk B: SubjectRecord (84 B each)
- Chunk I: B+ Tree pages (4096 B each)

FileHeader: magic ("REC1"), version (1), headerSize (256), next IDs, reserved.

---

## 3. Build Instructions

### Prerequisites

| Platform | Requirements |
|----------|--------------|
| Linux | GCC 10+, CMake 3.20+, Ninja, OpenGL, GLFW |
| macOS | Clang 15+, CMake 3.20+, Ninja, OpenGL |
| Windows | MinGW-w64 (GCC 10+), CMake 3.20+, Ninja, OpenGL, GLFW |

### Build Commands

```bash
# Configure with Ninja (build tree in builds/, output in builds/compiled_executable/)
cmake -G Ninja -S . -B builds

# Build
cmake --build builds
```

Build uses ccache if available. The `pack_scripts` tool compiles first, then runs to pack all Lua scripts into `builds/compiled_executable/scripts.bin` on every build.

### Alternative: Makefiles

```bash
cmake -S . -B builds
cmake --build builds
```

---

## 4. Running the Application

### GUI Mode

```bash
./builds/compiled_executable/TPAEDSIII
```

Expected behavior:
- Opens 800x600 window titled "AEDS III - Sistema de Matricula"
- Login screen with CPF + password fields
- Navigation sidebar on left, content area on right

### Sample Credentials

| Role | CPF | Password |
|------|-----|----------|
| Teacher | 00000000000 | 1234 |
| Student | 11111111111 | 1234 |

Sample data (teacher, student, subject) is auto-populated on first run when the database is empty. Delete `data/records.dat` to reset.

---

## 5. Running Tests

11 unit tests covering CRUD, enrollment, grade update, login, and edge cases.

### Build and Run Tests

```bash
cmake --build builds --target run_tests
./builds/compiled_executable/run_tests
```

Expected output:
```
=== DataManager Tests ===

[PASS] test_initialize
[PASS] test_create_student
[PASS] test_read_student
[PASS] test_read_nonexistent
[PASS] test_list_all_students
[PASS] test_soft_delete
[PASS] test_create_teacher_and_subject
[PASS] test_enrollment
[PASS] test_update_grade
[PASS] test_enrollments_by_student
[PASS] test_login

============================
Total: 11 | Pass: 11 | Fail: 0
============================
ALL TESTS PASSED!
```

Each test creates a fresh `data/test/` directory (cleaned up on start).

---

## 6. Project Structure

### Source Files

| Path | Description |
|------|-------------|
| `src/main.cpp` | Application entry point |
| `src/model/Record.hpp` | Packed binary structures + memcpy serialization |
| `src/model/Record.cpp` | Record toBytes/fromBytes helpers |
| `src/model/BPlusTree.hpp` | B+ Tree indexed storage (header) |
| `src/model/BPlusTree.cpp` | B+ Tree implementation (insert, search, erase, range) |
| `src/model/Student.hpp` | Student model wrapper |
| `src/model/Teacher.hpp` | Teacher model wrapper |
| `src/model/Subject.hpp` | Subject model wrapper |
| `src/model/Enrollment.hpp` | Enrollment model wrapper (BTreeLeafValue) |
| `src/controller/DataManager.hpp` | Main controller header |
| `src/controller/DataManager.cpp` | CRUD + login + enrollment logic |
| `src/controller/FileManager.hpp` | Low-level binary file I/O header |
| `src/controller/FileManager.cpp` | Read/write/seek file operations |
| `src/controller/IndexCtrl.hpp` | Hash-map index header |
| `src/controller/IndexCtrl.cpp` | DJB2 hash, persistence, rebuild |
| `src/view/ScriptArchive.hpp` | Binary Lua script archive header |
| `src/view/ScriptArchive.cpp` | Load .lua files, register via Lua require |
| `src/view/bindings/ImguiBindings.cpp` | Custom ImGui->Lua bindings |
| `src/utility/Constants.hpp` | Compile-time size/layout constants |
| `src/utility/Enums.hpp` | Enum definitions (RecStatus, RecType) |

### Build Output

| Path | Description |
|------|-------------|
| `builds/compiled_executable/TPAEDSIII` | GUI executable |
| `builds/compiled_executable/run_tests` | Test executable |
| `builds/compiled_executable/scripts.bin` | Packed Lua scripts archive |
| `builds/compiled_executable/pack_scripts` | Script packing tool |
| `builds/compiled_executable/assets/` | Copied assets (fonts) |

### Data Files (runtime)

| Path | Description |
|------|-------------|
| `data/records.dat` | Binary records file (header + chunks) |
| `data/records.idx` | Hash index file (persisted unordered_map) |

### Library Dependencies

| Path | Source |
|------|--------|
| `libs/hello_imgui/` | https://github.com/pthom/hello_imgui.git |
| `libs/lua/` | https://github.com/marovira/lua.git |
| `libs/luaaa/` | https://github.com/gengyong/luaaa.git |

---

## 7. Code Conventions

### C++ Style

- **Standard**: C++20 (`set(CMAKE_CXX_STANDARD 20)`)
- **Include Guards**: `#pragma once`
- **Namespaces**: `project_model`, `project_controller`, `project_view`, `project_utility`
- **Naming**:
  - Classes: `PascalCase` (e.g., `DataManager`, `BPlusTree`)
  - Methods: `camelCase` (e.g., `initialize`, `createStudent`)
  - Members: `_camelCase` (e.g., `dataFile_`, `lastError_`)
- **Memory**: Composition over heap allocation; `std::optional` for nullable returns
- **Integer Types**: `<cstdint>` (`int32_t`, `uint8_t`, `uint16_t`, `uint32_t`, `size_t`)
- **Persistence**: `#pragma pack(push,1)` + memcpy serialization via `serializeRecord<T>()` / `deserializeRecord<T>()`

### C++20 Features Used

- Range-based for loops with `const auto&`
- `auto` type deduction throughout
- `std::ignore` for unused parameters
- `constexpr` constants page geometry (PAGE_TYPE_OFF, NUM_KEYS_OFF, etc.)
- `[[maybe_unused]]` where appropriate
- `std::optional<T>` for nullable return values
- `requires std::is_trivially_copyable_v<T>` on serialization templates

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
- No `std::string` in persistable structs; use `char[]` + `nameStr()` helpers
- Logical deletion via `char status` flag (`'A'` = active, `'*'` = deleted)
- Generic memcpy serialization for any trivially copyable type

### Lua Binding Conventions

All DataManager methods are registered via `luaaa::LuaModule` (not `LuaClass`) to avoid the `MemberFunctionCaller` bug where `skip=1` shifts stack arguments. A `DataManager*` is pushed as a light userdata with a metatable whose `__index` points to the module table. This ensures `NonMemberFunctionCaller` (skip=0) is used, so `DM*` is correctly at stack position 1.

---

## 8. Key Facts

| Item | Value |
|------|-------|
| StudentRecord size | 141 B |
| TeacherRecord size | 167 B |
| SubjectRecord size | 84 B |
| BTreeLeafValue size | 25 B |
| B+ Tree leaf entry | 45 B (20 key + 25 value) |
| B+ Tree leaf max keys | 90 |
| B+ Tree leaf min keys | 45 |
| B+ Tree page size | 4096 B |
| FileHeader size | 256 B |
| ChunkInfo size | 32 B |
| Initial chunk capacity | 100 |
| Grade threshold | 60 / 100 (uint8_t) |
| Unit tests | 11 |
| Window title | "AEDS III - Sistema de Matricula" |
| Header magic | "REC1" (0x52454331) |
| Index magic | "INDE" (0x494E4445) |
| `klassName` bug fix | Changed to `className` in libs/luaaa/luaaa.hpp |

---

## 9. Extending the System

### Adding a New Entity

1. Define packed struct in `model/Record.hpp` (`#pragma pack(push,1)`)
2. Create model wrapper in `model/<Name>.hpp` with `serialize()` / `fromBytes()` / `raw()`
3. Add CRUD methods to `controller/DataManager`
4. Add LuaStack specialization in `main.cpp`
5. Register method in `dmMethods.fun()` lambda block

### Adding a New View

1. Create `.lua` file in `view/scripts/GUI/functional_views/` (action views) or `displayer_views/` (display views)
2. Module must export a `render()` function
3. Register in `view/scripts/handlers/router.lua`:

```lua
local NewView = require("GUI.functional_views.NewView")

-- In renderContent():
elseif currentView == "new_view" then
    NewView.render()
end
```

### Registering Lua Scripts

Scripts are packed into `scripts.bin` automatically by the `pack_scripts_always` CMake custom target, which scans `src/view/scripts/` recursively. No recompilation is needed — just create `.lua` files under the scripts directory and rebuild.

### Binding C++ to Lua

```cpp
luaaa::LuaModule dmMethods(L, "dm_methods");
dmMethods.fun("myMethod", [](DM* dm, int arg) { return dm->myMethod(arg); });
```

All registered functions receive `DM*` as first argument (via light userdata + __index).

---

## 10. Troubleshooting

### CMake: Generator Mismatch

**Error**: `Does not match the generator used previously`

**Fix**: Clear CMake cache
```bash
rm -rf builds/CMakeCache.txt builds/CMakeFiles
```

### CMake: Missing Dependencies

**Error**: `Could not find GLFW`

**Fix**: Install system dependencies
- Linux: `sudo apt install libglfw3-dev libopengl-dev`
- macOS: `brew install glfw`
- Windows: MinGW includes GLFW (via MSYS2)

### Lua: Script Not Found

**Error**: scripts.bin loads but specific require fails

**Fix**: Verify the `.lua` file exists under `src/view/scripts/`. The `pack_scripts` tool walks the directory recursively. Run `./builds/compiled_executable/pack_scripts src/view/scripts builds/compiled_executable/scripts.bin` manually to inspect.

### Data File Corruption

Delete `data/records.dat` and `data/records.idx` to start fresh.

---

## Quick Reference

| Command | Action |
|---------|--------|
| `cmake -G Ninja -S . -B builds` | Configure |
| `cmake --build builds` | Build |
| `./builds/compiled_executable/TPAEDSIII` | Run GUI |
| `./builds/compiled_executable/run_tests` | Run Tests |
| `rm -rf builds/CMakeCache.txt builds/CMakeFiles` | Clean Cache |

---

*This document is maintained with the project. Last updated: 2026-06-14*

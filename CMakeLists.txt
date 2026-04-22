cmake_minimum_required(VERSION 3.20)

project(TP_AEDSIII LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ======================
# Build Speed Optimizations
# ======================
# Persist FetchContent dependencies to avoid re-downloads on clean builds
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/.deps_cache)

# Enable ccache for 80%+ faster rebuilds
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
	message(STATUS "Using ccache for faster builds")
	set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
	set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

include(FetchContent)

# ======================
# Platform Detection
# ======================
message(STATUS "Building for platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Using generator: ${CMAKE_GENERATOR}")

# ======================
# FetchContent - Todas libs de fontes locais
# ======================
set(FETCHCONTENT_QUIET FALSE)
set(LOCAL_LIBS_DIR "${CMAKE_SOURCE_DIR}/libs")

# hello_imgui options (MUST be set BEFORE FetchContent_MakeAvailable)
set(HELLOIMGUI_BUILD_AUTOMATION_TEST OFF)
set(HELLOIMGUI_ADD_APP_WITH_INSTALL OFF)
set(HELLOIMGUI_USE_FREETYPE_PLUTOSVG OFF)
set(HELLOIMGUI_USE_SDL2 OFF)
set(HELLOIMGUI_USE_GLFW3 ON)
set(HELLOIMGUI_HAS_VULKAN OFF)
set(HELLOIMGUI_HAS_OPENGL3 ON)

FetchContent_Declare(hello_imgui
	GIT_REPOSITORY https://github.com/pthom/hello_imgui.git
	GIT_TAG        v1.5.0
)
FetchContent_MakeAvailable(hello_imgui)

FetchContent_Declare(lua
	GIT_REPOSITORY https://github.com/marovira/lua.git
	GIT_TAG        master
)
FetchContent_MakeAvailable(lua)

FetchContent_Declare(luaaa
	GIT_REPOSITORY https://github.com/gengyong/luaaa.git
	GIT_TAG        master
)
FetchContent_MakeAvailable(luaaa)

# ======================
# Backend OpenGL por plataforma
# ======================
if(WIN32)
	find_package(OpenGL REQUIRED)
	set(PLATFORM_LIBS OpenGL::GL)
elseif(APPLE)
	find_package(OpenGL REQUIRED)
	set(PLATFORM_LIBS OpenGL::GL)
elseif(UNIX)
	find_package(OpenGL REQUIRED)
	set(PLATFORM_LIBS OpenGL::GL)
endif()

# ======================
# Funcoes CMake do hello_imgui
# ======================
set(HELLO_IMGUI_CMAKE_PATH "${CMAKE_SOURCE_DIR}/libs/hello_imgui/hello_imgui_cmake")
include(${HELLO_IMGUI_CMAKE_PATH}/hello_imgui_add_app.cmake)
include(${HELLO_IMGUI_CMAKE_PATH}/hello_imgui_build_lib.cmake)

# ======================
# Cria diretorios de runtime
# ======================
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/data)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/src/views)

# ======================
# Configuracao de build
# ======================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/builds)

# Fontes C++ (estrutura flat: utility/, model/, controller/, view/)
set(APP_SOURCES
	${CMAKE_SOURCE_DIR}/src/main.cpp
	${CMAKE_SOURCE_DIR}/src/view/ImguiBindings.cpp
	${CMAKE_SOURCE_DIR}/src/model/Record.cpp
	${CMAKE_SOURCE_DIR}/src/controller/FileManager.cpp
	${CMAKE_SOURCE_DIR}/src/controller/IndexCtrl.cpp
	${CMAKE_SOURCE_DIR}/src/controller/DataManager.cpp
)

# ======================
# Aplicacao principal com GUI
# ======================
hello_imgui_add_app(TP_AEDSIII ${APP_SOURCES})
target_include_directories(TP_AEDSIII PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/libs/luaaa
	${CMAKE_SOURCE_DIR}/libs/lua/src
	${CMAKE_SOURCE_DIR}/libs/hello_imgui/src
)
target_link_libraries(TP_AEDSIII PRIVATE
	lua
	hello-imgui::hello_imgui
	${PLATFORM_LIBS}
)

# ======================
# Testes (CLI)
# ======================
set(TEST_SOURCES
	${CMAKE_SOURCE_DIR}/tests/test_main.cpp
	${CMAKE_SOURCE_DIR}/src/model/Record.cpp
	${CMAKE_SOURCE_DIR}/src/controller/FileManager.cpp
	${CMAKE_SOURCE_DIR}/src/controller/IndexCtrl.cpp
	${CMAKE_SOURCE_DIR}/src/controller/DataManager.cpp
)

add_executable(run_tests ${TEST_SOURCES})
target_include_directories(run_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

# ======================
# Flags do compilador
# ======================
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	add_compile_options(/W4 /permissive-)
endif()

message(STATUS "Build configuration complete!")

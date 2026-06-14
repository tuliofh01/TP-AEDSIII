cmake_minimum_required(VERSION 3.20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(TPAEDSIII LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ======================
# Build Speed Optimizations
# ======================
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
# Dependencies - libs/ folder (pre-cloned repos, no internet download)
# ======================
set(LOCAL_LIBS_DIR "${CMAKE_SOURCE_DIR}/libs")
set(FETCHCONTENT_SOURCE_DIR_HELLO_IMGUI ${LOCAL_LIBS_DIR}/hello_imgui)
set(FETCHCONTENT_SOURCE_DIR_LUA ${LOCAL_LIBS_DIR}/lua)
set(FETCHCONTENT_SOURCE_DIR_LUAAA ${LOCAL_LIBS_DIR}/luaaa)
set(FETCHCONTENT_QUIET FALSE)

# hello_imgui options
set(HELLOIMGUI_BUILD_AUTOMATION_TEST OFF)
set(HELLOIMGUI_ADD_APP_WITH_INSTALL OFF)
set(HELLOIMGUI_USE_FREETYPE OFF)
set(HELLOIMGUI_USE_FREETYPE_PLUTOSVG OFF)
set(HELLOIMGUI_USE_FREETYPE_LUNASVG OFF)
set(HELLOIMGUI_USE_SDL2 OFF)
set(HELLOIMGUI_USE_GLFW3 ON)
set(HELLOIMGUI_HAS_VULKAN OFF)
set(HELLOIMGUI_HAS_OPENGL3 ON)

FetchContent_Declare(hello_imgui
	GIT_REPOSITORY https://github.com/pthom/hello_imgui.git
	GIT_TAG        v1.5.0
	SOURCE_DIR     ${LOCAL_LIBS_DIR}/hello_imgui
)
FetchContent_MakeAvailable(hello_imgui)

FetchContent_Declare(lua
	GIT_REPOSITORY https://github.com/marovira/lua.git
	GIT_TAG        master
	SOURCE_DIR     ${LOCAL_LIBS_DIR}/lua
)
FetchContent_MakeAvailable(lua)

FetchContent_Declare(luaaa
	GIT_REPOSITORY https://github.com/gengyong/luaaa.git
	GIT_TAG        master
	SOURCE_DIR     ${LOCAL_LIBS_DIR}/luaaa
)
FetchContent_MakeAvailable(luaaa)

# ======================
# Backend OpenGL (todas as plataformas)
# ======================
find_package(OpenGL REQUIRED)
set(PLATFORM_LIBS OpenGL::GL)

# ======================
# Funcoes CMake do hello_imgui
# ======================
set(HELLO_IMGUI_CMAKE_PATH "${CMAKE_SOURCE_DIR}/libs/hello_imgui/hello_imgui_cmake")
include(${HELLO_IMGUI_CMAKE_PATH}/hello_imgui_add_app.cmake)
include(${HELLO_IMGUI_CMAKE_PATH}/hello_imgui_build_lib.cmake)

# ======================
# Runtime directories
# ======================
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/data)

# ======================
# Build output directory
# ======================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/builds/compiled_executable)

# Copy assets/ to output directory at configure time
if(EXISTS ${CMAKE_SOURCE_DIR}/assets)
	file(COPY ${CMAKE_SOURCE_DIR}/assets/ DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets)
endif()

# ======================
# Script packing tool
# ======================
add_executable(pack_scripts
	${CMAKE_SOURCE_DIR}/tools/pack_scripts.cpp
	${CMAKE_SOURCE_DIR}/src/view/ScriptArchive.cpp
)
target_include_directories(pack_scripts PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/libs/lua/src
)
target_link_libraries(pack_scripts PRIVATE liblua)

# ======================
# C++ sources (GUI app)
# ======================
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

# ======================
# Main application with GUI
# ======================
hello_imgui_add_app(TPAEDSIII ${APP_SOURCES})
	target_include_directories(TPAEDSIII PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/libs/luaaa
	${CMAKE_SOURCE_DIR}/libs/lua/src
	${CMAKE_SOURCE_DIR}/libs/hello_imgui/src
)
target_link_libraries(TPAEDSIII PRIVATE
	liblua
	hello-imgui::hello_imgui
	${PLATFORM_LIBS}
)

# ======================
# Rebuild scripts.bin every build (reflects current script files)
# ======================
# The custom target always runs, ensuring deleted .lua files are excluded
add_custom_target(pack_scripts_always ALL
	COMMAND $<TARGET_FILE:pack_scripts>
		${CMAKE_SOURCE_DIR}/src/view/scripts
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts.bin
	DEPENDS pack_scripts
	COMMENT "Packing Lua scripts into ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts.bin"
)
add_dependencies(TPAEDSIII pack_scripts_always)

# ======================
# Tests (CLI)
# ======================
set(TEST_SOURCES
	${CMAKE_SOURCE_DIR}/tests/test_main.cpp
	${CMAKE_SOURCE_DIR}/src/model/Record.cpp
	${CMAKE_SOURCE_DIR}/src/model/BPlusTree.cpp
	${CMAKE_SOURCE_DIR}/src/controller/FileManager.cpp
	${CMAKE_SOURCE_DIR}/src/controller/IndexCtrl.cpp
	${CMAKE_SOURCE_DIR}/src/controller/DataManager.cpp
)

add_executable(run_tests ${TEST_SOURCES})
target_include_directories(run_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

# ======================
# Compiler flags
# ======================
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	add_compile_options(/W4 /permissive-)
endif()

message(STATUS "Build configuration complete!")

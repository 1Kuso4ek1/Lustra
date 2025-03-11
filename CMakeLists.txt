cmake_minimum_required(VERSION 3.10)
project(Engine)

find_package(Threads REQUIRED)

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)

set(LLGL_BUILD_EXAMPLES OFF)
set(LLGL_BUILD_RENDERER_NULL OFF)
set(LLGL_BUILD_RENDERER_OPENGL ON)
set(LLGL_GL_ENABLE_DSA_EXT ON)
set(LLGL_GL_ENABLE_VENDOR_EXT ON)
set(LLGL_GL_INCLUDE_EXTERNAL ON)

set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_WARNINGS_AS_ERRORS OFF)

set(BUILD_DOC OFF)
set(BUILD_SANDBOX OFF)
set(SKIP_PERFORMANCE_COMPARISON ON)

set(OUTPUT_DIR ${CMAKE_BINARY_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR} CACHE STRING "" FORCE)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR} CACHE STRING "" FORCE)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR} CACHE STRING "" FORCE)

file(GLOB ENGINE_SOURCES "src/*.cpp" "src/*/*.cpp" "src/*/*/*.cpp")
file(GLOB IMGUI_SOURCES "libraries/imgui/*.cpp")
file(GLOB IMGUIZMO_SOURCES "libraries/ImGuizmo/*.cpp")

list(REMOVE_ITEM ENGINE_SOURCES "src/Editor/main.cpp")
list(REMOVE_ITEM ENGINE_SOURCES "src/Launcher/main.cpp")

set(
    ENGINE_INCLUDE_DIRS
    
    include
    include/Assets
    include/Assets/Loaders
    include/Audio
    include/Components
    include/Core
    include/Events
    include/Graphics
    include/ImGui
    include/Input
    include/Physics
    include/Scene
    include/Scripting

    include/Editor
    include/Launcher
)

set(
    EXTERNAL_INCLUDE_DIRS

    libraries/stb
    libraries/imgui
    libraries/imgui/backends
    libraries/ImGuizmo
    libraries/glfw/include
    libraries/LLGL/include
    libraries/entt/src
    libraries/assimp/include
    libraries/JoltPhysics
    libraries/miniaudio

    libraries/Angelscript/sdk/angelscript/include
    
    libraries/Angelscript/sdk/add_on/scriptbuilder
    libraries/Angelscript/sdk/add_on/scriptstdstring
    libraries/Angelscript/sdk/add_on/scriptarray
    libraries/Angelscript/sdk/add_on/autowrapper
    libraries/Angelscript/sdk/add_on/scriptmath
    libraries/Angelscript/sdk/add_on/datetime
    libraries/Angelscript/sdk/add_on/scriptdictionary
    libraries/Angelscript/sdk/add_on/scriptfile
)

set(ANGELSCRIPT_LIBRARY_NAME angelscript)
if(APPLE)
    set(ANGELSCRIPT_LIBRARY_NAME Angelscript)
endif()

set(CMAKE_DEBUG_POSTFIX "" CACHE STRING "" FORCE)

set(
    INCLUDE_DIRS

    ${EXTERNAL_INCLUDE_DIRS}
    ${ENGINE_INCLUDE_DIRS}
)

set(
    IMGUI_BACKEND_SOURCES
    
    libraries/imgui/backends/imgui_impl_glfw.cpp
    libraries/imgui/backends/imgui_impl_opengl3.cpp

    libraries/imgui/misc/cpp/imgui_stdlib.cpp
)

set(
    ANGELSCRIPT_ADD_ON_SOURCES

    libraries/Angelscript/sdk/add_on/scriptbuilder/scriptbuilder.cpp
	libraries/Angelscript/sdk/add_on/scriptstdstring/scriptstdstring.cpp
	libraries/Angelscript/sdk/add_on/scriptarray/scriptarray.cpp
	libraries/Angelscript/sdk/add_on/scriptmath/scriptmath.cpp
	libraries/Angelscript/sdk/add_on/datetime/datetime.cpp
	libraries/Angelscript/sdk/add_on/scriptdictionary/scriptdictionary.cpp
	libraries/Angelscript/sdk/add_on/scriptfile/scriptfile.cpp
)

add_library(
    Engine

    ${ENGINE_SOURCES}
    ${IMGUI_SOURCES}
    ${IMGUI_BACKEND_SOURCES}
    ${IMGUIZMO_SOURCES}
    ${ANGELSCRIPT_ADD_ON_SOURCES}
)

target_include_directories(Engine PUBLIC ${INCLUDE_DIRS})
target_link_libraries(
    Engine

    assimp
    LLGL
    Threads::Threads
    stdc++fs
    glm
    glfw
    Jolt
    ${ANGELSCRIPT_LIBRARY_NAME}
    miniaudio
)

add_executable(
    Editor

    src/Editor/main.cpp
)

add_definitions("-DEDITOR_ROOT=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

add_executable(
    Launcher

    src/Launcher/main.cpp
)

set_target_properties(Editor Launcher Engine PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)

include_directories(${INCLUDE_DIRS})

target_include_directories(Editor PUBLIC include)
target_link_libraries(Editor Engine)

target_include_directories(Launcher PUBLIC include)
target_link_libraries(Launcher Engine)

add_subdirectory(libraries/glfw)
add_subdirectory(libraries/glm)
add_subdirectory(libraries/LLGL)
add_subdirectory(libraries/assimp)
add_subdirectory(libraries/JoltPhysics/Build)
add_subdirectory(libraries/Angelscript/sdk/angelscript/projects/cmake)
add_subdirectory(libraries/cereal)
add_subdirectory(libraries/miniaudio)

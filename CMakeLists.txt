cmake_minimum_required(VERSION 3.10)
project(LLGLTest)

find_package(Threads REQUIRED)

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)

set(LLGL_BUILD_EXAMPLES OFF)
set(LLGL_BUILD_RENDERER_NULL OFF)
set(LLGL_BUILD_RENDERER_OPENGL ON)
set(LLGL_GL_ENABLE_DSA_EXT ON)

set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_WARNINGS_AS_ERRORS OFF)

set(OUTPUT_DIR ${CMAKE_BINARY_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR} CACHE STRING "" FORCE)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR} CACHE STRING "" FORCE)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR} CACHE STRING "" FORCE)

file(GLOB ENGINE_SOURCES "src/*.cpp" "src/*/*.cpp" "src/*/*/*.cpp")
file(GLOB IMGUI_SOURCES "libraries/imgui/*.cpp")
file(GLOB IMGUIZMO_SOURCES "libraries/ImGuizmo/*.cpp")

set(
    ENGINE_INCLUDE_DIRS
    
    include
    include/Assets
    include/Assets/Loaders
    include/Core
    include/Events
    include/Graphics
    include/ImGui
    include/Input
    include/Scene
    include/TestApps
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
)

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

add_library(
    Engine

    ${ENGINE_SOURCES}
    ${IMGUI_SOURCES}
    ${IMGUI_BACKEND_SOURCES}
    ${IMGUIZMO_SOURCES}
)

target_include_directories(Engine PUBLIC ${INCLUDE_DIRS})
target_link_libraries(Engine assimp LLGL Threads::Threads stdc++fs glm glfw)

add_executable(
    LLGLTest

    main.cpp
)

set_target_properties(LLGLTest Engine PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)

include_directories(${INCLUDE_DIRS})

target_include_directories(LLGLTest PUBLIC include)
target_link_libraries(LLGLTest Engine)

add_subdirectory(libraries/glfw)
add_subdirectory(libraries/glm)
add_subdirectory(libraries/LLGL)
add_subdirectory(libraries/assimp)

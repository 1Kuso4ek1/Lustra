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

add_subdirectory(libraries/glfw)
add_subdirectory(libraries/glm)
add_subdirectory(libraries/LLGL)

set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)

file(GLOB ENGINE_SOURCES "src/*.cpp" "src/*/*.cpp" "src/*/*/*.cpp")
file(GLOB IMGUI_SOURCES "libraries/imgui/*.cpp")
file(GLOB ENGINE_INCLUDE_DIRS "include" "include/*" "include/*/*")

set(
    INCLUDE_DIRS

    libraries/stb
    libraries/imgui
    libraries/imgui/backends
    libraries/glfw/include
    libraries/LLGL/include
    libraries/entt/src

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
)

target_include_directories(Engine PUBLIC ${INCLUDE_DIRS})
target_link_libraries(Engine LLGL Threads::Threads stdc++fs glm glfw)

add_executable(
    LLGLTest

    main.cpp
)

set_target_properties(LLGLTest Engine PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)

include_directories(${INCLUDE_DIRS})

target_include_directories(LLGLTest PUBLIC include)
target_link_libraries(LLGLTest Engine)

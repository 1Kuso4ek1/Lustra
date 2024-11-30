cmake_minimum_required(VERSION 3.10)
project(LLGLTest)

find_package(LLGL REQUIRED)
find_package(Threads REQUIRED)
find_package(glm REQUIRED)

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)

add_subdirectory(libraries/glfw)

#include(FindPkgConfig)

#pkg_search_module(glfw REQUIRED glfw)

file(GLOB ENGINE_SOURCES "src/*.cpp")
file(GLOB IMGUI_SOURCES "libraries/imgui/*.cpp")

file(
    GLOB
    IMGUI_BACKEND_SOURCES
    
    libraries/imgui/backends/imgui_impl_glfw.cpp
    libraries/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(
    LLGLTest

    main.cpp

    ${ENGINE_SOURCES}
    ${IMGUI_SOURCES}
    ${IMGUI_BACKEND_SOURCES}

    libraries/imgui/misc/cpp/imgui_stdlib.cpp
)

set_target_properties(LLGLTest PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)

include_directories(include libraries/stb libraries/imgui libraries/imgui/backends libraries/entt)

target_include_directories(LLGLTest PUBLIC libraries/stb libraries/imgui/backends include ${glfw_INCLUDE_DIRS})
target_link_libraries(LLGLTest LLGL::LLGL Threads::Threads stdc++fs glm::glm glfw)

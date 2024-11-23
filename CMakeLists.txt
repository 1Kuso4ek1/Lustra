cmake_minimum_required(VERSION 3.10)
project(LLGLTest)

find_package(LLGL REQUIRED)
find_package(Threads REQUIRED)
find_package(glm REQUIRED)

include(FindPkgConfig)

pkg_search_module(glfw REQUIRED glfw)

add_executable(
    LLGLTest

    main.cpp

    src/Renderer.cpp
    src/Matrices.cpp
    src/Mesh.cpp
    src/Application.cpp
    src/CubeApp.cpp
    src/Multithreading.cpp
    src/TextureManager.cpp
    src/Window.cpp
    src/Keyboard.cpp
    src/Mouse.cpp

    libraries/imgui/imgui.cpp
    libraries/imgui/imgui_demo.cpp
    libraries/imgui/imgui_draw.cpp
    libraries/imgui/imgui_tables.cpp
    libraries/imgui/imgui_widgets.cpp

    libraries/imgui/backends/imgui_impl_glfw.cpp
    libraries/imgui/backends/imgui_impl_opengl3.cpp

    libraries/imgui/backends/imgui_impl_glfw.cpp
    libraries/imgui/backends/imgui_impl_opengl3.cpp

    libraries/imgui/misc/cpp/imgui_stdlib.cpp
)

set_target_properties(LLGLTest PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)

include_directories(include libraries/stb libraries/imgui libraries/imgui/backends)

target_include_directories(LLGLTest PUBLIC libraries/stb libraries/imgui/backends include ${glfw_INCLUDE_DIRS})
target_link_libraries(LLGLTest LLGL::LLGL Threads::Threads stdc++fs glm::glm glfw)

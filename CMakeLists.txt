cmake_minimum_required(VERSION 3.10)
project(LLGLTest)

find_package(LLGL REQUIRED)
find_package(Threads REQUIRED)
find_package(glm REQUIRED)

add_executable(LLGLTest main.cpp src/Renderer.cpp src/Matrices.cpp src/Mesh.cpp src/Application.cpp src/CubeApp.cpp src/Multithreading.cpp src/TextureManager.cpp)

set_target_properties(LLGLTest PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)

include_directories(include)

target_include_directories(LLGLTest PUBLIC include)
target_link_libraries(LLGLTest LLGL::LLGL Threads::Threads stdc++fs glm::glm)

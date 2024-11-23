#pragma once
#include <Application.hpp>
#include <TextureManager.hpp>
#include <Window.hpp>
#include <Keyboard.hpp>
#include <Mouse.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class CubeApp : public dev::Application
{
public:
    CubeApp();
    ~CubeApp();

    void Run() override;

private:
    void LoadShaders();
    void LoadTextures();
    void InitImGui();

    float degrees = 0.5f;

    dev::WindowPtr window;

    std::unique_ptr<dev::Mesh> mesh;

    LLGL::PipelineState* pipeline{};

    LLGL::Shader* vertexShader{};
    LLGL::Shader* fragmentShader{};

    std::shared_ptr<dev::TextureHandle> texture;
    LLGL::Sampler* sampler{};

    std::shared_ptr<dev::Matrices> matrices;
};

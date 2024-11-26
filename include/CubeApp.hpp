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
    void DestroyImGui();
    void NewImGuiFrame();
    void SetupImGuiStyle();
    void DrawImGui();
    void Draw();

    float degrees = 0.5f;

    glm::vec3 axis{ 0.0f, 1.0f, 0.0f };
    float angle = 0.0f;

    dev::WindowPtr window;

    dev::Timer keyboardTimer;

    std::unique_ptr<dev::Mesh> mesh;

    LLGL::PipelineState* pipeline{};

    LLGL::Shader* vertexShader{};
    LLGL::Shader* fragmentShader{};

    std::shared_ptr<dev::TextureHandle> texture;
    LLGL::Sampler* sampler{};

    std::shared_ptr<dev::Matrices> matrices;
};

#pragma once
#include <Application.hpp>
#include <TextureManager.hpp>
#include <Window.hpp>
#include <Keyboard.hpp>
#include <Mouse.hpp>

class CubeApp : public dev::Application
{
public:
    CubeApp();

    void Run() override;

private:
    void LoadShaders();
    void LoadTextures();

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

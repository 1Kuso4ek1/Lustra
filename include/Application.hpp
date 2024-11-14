#pragma once
#include "Matrices.hpp"
#include "Mesh.hpp"
#include <LLGL/PipelineState.h>
#include <Utils.hpp>
#include <Renderer.hpp>
#include <memory>

class Application
{
public:
    Application();
    ~Application();

    void Run();

private:    
    void LoadShaders();
    void LoadTextures();

    float degrees = 0.f;

    std::unique_ptr<Mesh> mesh;

    LLGL::PipelineState* pipeline{};

    LLGL::Shader* vertexShader{};
    LLGL::Shader* fragmentShader{};

    LLGL::Texture* texture{};
    LLGL::Sampler* sampler{};

    std::shared_ptr<Matrices> matrices;
};

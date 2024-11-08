#pragma once
#include <LLGL/Buffer.h>
#include <LLGL/LLGL.h>
#include <LLGL/Utils/VertexFormat.h>
#include <LLGL/Utils/Utility.h>

class Application
{
public:
    Application();
    ~Application();

    void Run();

private:
    void LoadRenderSystem(const LLGL::RenderSystemDescriptor& desc);
    void InitSwapChain(const LLGL::Extent2D& resolution, bool fullscreen = false, int samples = 1);
    void SetupVertexFormat();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void LoadShaders();
    void CreatePipeline();
    void CreateCommandBuffer();
    void LoadTextures();

    struct Vertex
    {
        float pos[2];
        float coords[2];
    };

    Vertex vertices[4] = {
        { { -1.0f, 1.0f },  { 0.0f, 0.0f } },
        { { 1.0f,  1.0f },  { 1.0f, 0.0f } },
        { { -1.0f, -1.0f }, { 0.0f, 1.0f } },
        { { 1.0f,  -1.0f }, { 1.0f, 1.0f } }
    };

    uint32_t indices[6] = { 0, 1, 2, 2, 1, 3 };

    LLGL::RenderSystemPtr renderSystem;

    LLGL::SwapChain* swapChain{};

    LLGL::VertexFormat vertexFormat;

    LLGL::Buffer* vertexBuffer{};
    LLGL::Buffer* indexBuffer{};

    LLGL::Shader* vertexShader{};
    LLGL::Shader* fragmentShader{};

    LLGL::PipelineState* pipeline{};

    LLGL::CommandBuffer* commandBuffer{};

    LLGL::Texture* texture{};
    
    LLGL::Sampler* sampler{};
};

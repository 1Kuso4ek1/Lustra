#pragma once
#include <LLGL/BufferFlags.h>
#include <LLGL/CommandBuffer.h>
#include <LLGL/LLGL.h>
#include <LLGL/PipelineState.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/Types.h>
#include <LLGL/Utils/VertexFormat.h>
#include <LLGL/Window.h>

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
    void LoadShaders();
    void CreatePipeline();
    void CreateCommandBuffer();

    struct Vertex
    {
        float pos[2];
        uint8_t color[4];
    };

    Vertex vertices[3] = {
        Vertex{ { 0.0f,  0.5f }, { 255, 0, 0, 255 } },
        Vertex{ { 0.5f, -0.5f }, { 0, 255, 0, 255 } },
        Vertex{ { -0.5f, -0.5f }, { 0, 0, 255, 255 } }
    };

    LLGL::RenderSystemPtr renderSystem;
    
    LLGL::SwapChainDescriptor swapChainDescriptor;

    LLGL::SwapChain* swapChain{};

    LLGL::VertexFormat vertexFormat;

    LLGL::BufferDescriptor vertexBufferDescriptor;
    LLGL::Buffer* vertexBuffer{};

    LLGL::ShaderDescriptor vertexShaderDescriptor;
    LLGL::ShaderDescriptor fragmentShaderDescriptor;

    LLGL::Shader* vertexShader{};
    LLGL::Shader* fragmentShader{};

    LLGL::GraphicsPipelineDescriptor pipelineDescriptor;
    LLGL::PipelineState* pipeline{};

    LLGL::CommandBuffer* commandBuffer{};
};

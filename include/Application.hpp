#pragma once
#include <LLGL/Buffer.h>
#include <LLGL/CommandQueue.h>
#include <LLGL/LLGL.h>
#include <LLGL/Utils/VertexFormat.h>
#include <LLGL/Utils/Utility.h>

#include <Matrices.hpp>
#include <memory>

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
    void CreateMatricesBuffer();
    
    void LoadShaders();
    void CreatePipeline();
    void CreateCommandBuffer();
    void LoadTextures();

    struct Vertex
    {
        float pos[3];
        float normal[3];
        float coords[2];
    };

    /*Vertex vertices[36] = {
        { { -1.0f, 1.0f, 0.0f },  { 0.0f, 0.0f } },
        { { 1.0f,  1.0f, 0.0f },  { 1.0f, 0.0f } },
        { { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },
        { { 1.0f,  -1.0f, 0.0f }, { 1.0f, 1.0f } }
    };*/

    Vertex vertices[24] = {
        { { -1, -1, -1 }, { 0, 0, -1 }, { 0, 1 } },
        { { -1, 1, -1 }, { 0, 0, -1 }, { 0, 0 } },
        { { 1, 1, -1 }, { 0, 0, -1 }, { 1, 0 } },
        { { 1, -1, -1 }, { 0, 0, -1 }, { 1, 1 } },

        { {  1, -1, -1 }, { 1, 0, 0 }, { 0, 1 } },
        { {  1, 1, -1 }, { 1, 0, 0 }, { 0, 0 } },
        { {  1, 1, 1 }, { 1, 0, 0 }, { 1, 0 } },
        { {  1, -1, 1 }, { 1, 0, 0 }, { 1, 1 } },

        { { -1, -1, 1 }, { -1, 0, 0 }, { 0, 1 } },
        { { -1, 1, 1 }, { -1, 0, 0 }, { 0, 0 } },
        { { -1, 1, -1 }, { -1, 0, 0 }, { 1, 0 } },
        { { -1, -1, -1 }, { -1, 0, 0 }, { 1, 1 } },

        { { -1, 1, -1 }, { 0, 1, 0 }, { 0, 1 } },
        { { -1, 1, 1 }, { 0, 1, 0 }, { 0, 0 } },
        { { 1, 1, 1 }, { 0, 1, 0 }, { 1, 0 } },
        { { 1, 1, -1 }, { 0, 1, 0 }, { 1, 1 } },

        { { -1, -1, 1 }, { 0, -1, 0 }, { 0, 1 } },
        { { -1, -1, -1 }, { 0, -1, 0 }, { 0, 0 } },
        { { 1, -1, -1 }, { 0, -1, 0 }, { 1, 0 } },
        { { 1, -1, 1 }, { 0, -1, 0 }, { 1, 1 } },

        { { 1, -1, 1 }, { 0, 0, 1 }, { 0, 1 } },
        { { 1, 1, 1 }, { 0, 0, 1 }, { 0, 0 } },
        { { -1, 1, 1 }, { 0, 0, 1 }, { 1, 0 } },
        { { -1, -1, 1 }, { 0, 0, 1 }, { 1, 1 } }
    };

    uint32_t indices[36] = {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23
    };

    //uint32_t indices[6] = { 0, 1, 2, 2, 1, 3 };

    LLGL::RenderSystemPtr renderSystem;

    LLGL::SwapChain* swapChain{};

    LLGL::VertexFormat vertexFormat;

    LLGL::Buffer* vertexBuffer{};
    LLGL::Buffer* indexBuffer{};
    LLGL::Buffer* matricesBuffer{};

    LLGL::Shader* vertexShader{};
    LLGL::Shader* fragmentShader{};

    LLGL::PipelineState* pipeline{};

    LLGL::CommandBuffer* commandBuffer{};

    LLGL::CommandQueue* commandQueue{};

    LLGL::Texture* texture{};
    
    LLGL::Sampler* sampler{};

    std::shared_ptr<Matrices> matrices;
};

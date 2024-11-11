#pragma once
#include <Utils.hpp>

#include <functional>
#include <unordered_map>

class Renderer
{
public: // Get the singleton instance
    static Renderer& Get();

public: // Public methods
    void InitSwapChain(const LLGL::Extent2D& resolution, bool fullscreen = false, int samples = 1);

    void RenderPass(std::function<void(LLGL::CommandBuffer*)> setupBuffers, // Set vert/ind/static buffers with CommandBuffer
                    std::unordered_map<uint32_t, LLGL::Resource*> resources, // A map of resources { binding, Resource_ptr }
                    std::function<void(LLGL::CommandBuffer*)> draw, // Call the draw function
                    LLGL::PipelineState* pipeline = nullptr); // If nullptr, use the default one

    void Present();

    LLGL::Buffer* CreateBuffer(const LLGL::BufferDescriptor& bufferDesc, const void* data = nullptr);

    LLGL::Window* GetWindow() const;
    LLGL::VertexFormat GetDefaultVertexFormat() const;
    LLGL::PipelineState* GetDefaultPipeline() const;

    bool IsInit(); // Will return false if RenderSystem init failed

private: // Singleton-related
    Renderer();

    std::unique_ptr<Renderer> instance;

private: // Private methods
    void LoadRenderSystem(const LLGL::RenderSystemDescriptor& desc);
    void SetupDefaultVertexFormat();
    void SetupDefaultPipeline();
    void SetupCommandBuffer();

private: // Private members
    LLGL::RenderSystemPtr renderSystem;

    LLGL::SwapChain* swapChain{};

    LLGL::PipelineState* defaultPipeline{};

    LLGL::CommandBuffer* commandBuffer{};

    LLGL::CommandQueue* commandQueue{}; // Unused for now

    LLGL::VertexFormat defaultVertexFormat;

    std::shared_ptr<Matrices> matrices;
};

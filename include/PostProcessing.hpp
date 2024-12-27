#pragma once
#include <Renderer.hpp>
#include <Mesh.hpp>

namespace dev
{

class PostProcessing
{
public:
    PostProcessing(const LLGL::PipelineLayoutDescriptor& layoutDesc, LLGL::GraphicsPipelineDescriptor pipelineDesc,
                       const LLGL::Extent2D& resolution = Renderer::Get().GetSwapChain()->GetResolution(), bool newRenderTarget = true);

    LLGL::Texture* Apply(const std::unordered_map<uint32_t, LLGL::Resource*>& resources,
                         std::function<void(LLGL::CommandBuffer*)> setUniforms,
                         LLGL::RenderTarget* renderTarget = nullptr);

    LLGL::Texture* GetFrame();
    LLGL::RenderTarget* GetRenderTarget();

protected:
    LLGL::Texture* frame;
    LLGL::RenderTarget* renderTarget;

    std::shared_ptr<Mesh> rect;
    LLGL::PipelineState* rectPipeline;
};

}

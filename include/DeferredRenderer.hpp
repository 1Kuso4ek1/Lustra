#pragma once
#include <Mesh.hpp>
#include <TextureManager.hpp>
#include <RendererBase.hpp>

namespace dev
{

class DeferredRenderer final : public RendererBase
{
public:
    DeferredRenderer(const LLGL::Extent2D& resolution = Renderer::Get().GetSwapChain()->GetResolution());

    void Draw(LLGL::RenderTarget* renderTarget = Renderer::Get().GetSwapChain()) override;

    LLGL::RenderTarget* GetPrimaryRenderTarget() override;

private:
    LLGL::Texture* gBufferPosition;
    LLGL::Texture* gBufferAlbedo;
    LLGL::Texture* gBufferNormal;
    LLGL::Texture* gBufferDepth;

    LLGL::RenderTarget* gBuffer;
    LLGL::PipelineState* gBufferPipeline;

    LLGL::PipelineState* lightingPipeline;

    LLGL::PipelineState* rectPipeline;

    std::shared_ptr<Mesh> rect;
};

}

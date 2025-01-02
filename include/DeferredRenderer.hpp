#pragma once
#include <Mesh.hpp>
#include <RendererBase.hpp>
#include <EventManager.hpp>
#include <TextureAsset.hpp>
#include <AssetManager.hpp>

namespace dev
{

class DeferredRenderer final : public RendererBase, public EventListener
{
public:
    DeferredRenderer(const LLGL::Extent2D& resolution = Renderer::Get().GetSwapChain()->GetResolution(), bool registerEvent = true);
    ~DeferredRenderer();

    void Draw(
        const std::unordered_map<uint32_t, LLGL::Resource*>& resources,
        std::function<void(LLGL::CommandBuffer*)> setUniforms,
        LLGL::RenderTarget* renderTarget = Renderer::Get().GetSwapChain()
    ) override;

    void OnEvent(Event& event) override;

    LLGL::RenderTarget* GetPrimaryRenderTarget() override;

private:
    LLGL::Texture* gBufferPosition;
    LLGL::Texture* gBufferAlbedo;
    LLGL::Texture* gBufferNormal;
    LLGL::Texture* gBufferDepth;

    LLGL::RenderTarget* gBuffer;
    LLGL::PipelineState* gBufferPipeline;

    std::shared_ptr<Mesh> rect;
    LLGL::PipelineState* rectPipeline;
};

}

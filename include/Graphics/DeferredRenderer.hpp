#pragma once
#include <Mesh.hpp>
#include <RendererBase.hpp>
#include <EventManager.hpp>
#include <TextureAsset.hpp>
#include <AssetManager.hpp>
#include <ModelAsset.hpp>
#include <ShaderAsset.hpp>
#include <Window.hpp>

namespace lustra
{

class DeferredRenderer final : public RendererBase, public EventListener, public Singleton<DeferredRenderer>
{
public:
    ~DeferredRenderer() override;

    void Draw(
        const std::unordered_map<uint32_t, LLGL::Resource*>& resources,
        std::function<void(LLGL::CommandBuffer*)> setUniforms,
        LLGL::RenderTarget* renderTarget = Renderer::Get().GetSwapChain()
    ) override;

    void OnEvent(Event& event) override;

    LLGL::RenderTarget* GetPrimaryRenderTarget() override;
    LLGL::Texture* GetDepth() override;

    LLGL::Texture* GetPosition() const;
    LLGL::Texture* GetAlbedo() const;
    LLGL::Texture* GetNormal() const;
    LLGL::Texture* GetCombined() const;
    LLGL::Texture* GetEmission() const;

private:
    DeferredRenderer();

    friend class Singleton<DeferredRenderer>;

private:
    FragmentShaderAssetPtr lightingPass;

    LLGL::PipelineLayoutDescriptor layoutDesc;

    LLGL::Texture* gBufferPosition;
    LLGL::Texture* gBufferAlbedo;
    LLGL::Texture* gBufferNormal;
    LLGL::Texture* gBufferCombined;
    LLGL::Texture* gBufferEmission;
    LLGL::Texture* gBufferDepth;

    LLGL::RenderTarget* gBuffer;
    LLGL::PipelineState* gBufferPipeline{};

    MeshPtr rect;
    LLGL::PipelineState* rectPipeline;
};

}

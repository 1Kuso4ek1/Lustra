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

class DeferredRenderer : public RendererBase, public EventListener, public Singleton<DeferredRenderer>
{
public:
    ~DeferredRenderer();

    void Draw(
        const std::unordered_map<uint32_t, LLGL::Resource*>& resources,
        std::function<void(LLGL::CommandBuffer*)> setUniforms,
        LLGL::RenderTarget* renderTarget = Renderer::Get().GetSwapChain()
    ) override;

    void OnEvent(Event& event) override;

    LLGL::RenderTarget* GetPrimaryRenderTarget() override;
    LLGL::Texture* GetDepth() override;

    LLGL::Texture* GetPosition();
    LLGL::Texture* GetAlbedo();
    LLGL::Texture* GetNormal();
    LLGL::Texture* GetCombined();
    LLGL::Texture* GetEmission();

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
    LLGL::PipelineState* gBufferPipeline;

    MeshPtr rect;
    LLGL::PipelineState* rectPipeline;
};

}

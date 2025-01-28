#pragma once
#include <Renderer.hpp>
#include <Mesh.hpp>
#include <EventManager.hpp>
#include <AssetManager.hpp>
#include <ModelAsset.hpp>

namespace dev
{

class PostProcessing : public EventListener
{
public:
    PostProcessing(
        const LLGL::PipelineLayoutDescriptor& layoutDesc, LLGL::GraphicsPipelineDescriptor pipelineDesc,
        const LLGL::Extent2D& resolution = Renderer::Get().GetSwapChain()->GetResolution(),
        bool newRenderTarget = true,
        bool registerEvent = true,
        const LLGL::Format& format = LLGL::Format::RGBA16Float
    );
    ~PostProcessing();

    void OnEvent(Event& event) override;

    LLGL::Texture* Apply(
        const std::unordered_map<uint32_t, LLGL::Resource*>& resources,
        std::function<void(LLGL::CommandBuffer*)> setUniforms,
        LLGL::RenderTarget* renderTarget = nullptr
    );

    LLGL::Texture* GetFrame();
    LLGL::RenderTarget* GetRenderTarget();

protected:
    LLGL::TextureDescriptor frameDesc;
    
    LLGL::Texture* frame;
    LLGL::RenderTarget* renderTarget;

    MeshPtr rect;
    LLGL::PipelineState* rectPipeline;
};

using PostProcessingPtr = std::shared_ptr<PostProcessing>;

}

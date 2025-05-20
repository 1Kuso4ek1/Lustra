#pragma once
#include <AssetManager.hpp>
#include <EventManager.hpp>
#include <Mesh.hpp>
#include <Renderer.hpp>
#include <ShaderAsset.hpp>
#include <Window.hpp>

namespace lustra
{

class PostProcessing final : public EventListener
{
public:
    // Make it use something like PostProcessingDescriptor?
    PostProcessing(
        const LLGL::PipelineLayoutDescriptor& layoutDesc,
        //const LLGL::GraphicsPipelineDescriptor& pipelineDesc,
        const VertexShaderAssetPtr& vertexShader,
        const FragmentShaderAssetPtr& fragmentShader,
        const LLGL::Extent2D& resolution = Renderer::Get().GetViewportResolution(),

        bool newRenderTarget = true,
        bool registerEvent = true,
        bool mipMaps = false,

        const LLGL::Format& format = LLGL::Format::RGBA16Float
    );
    ~PostProcessing() override;

    void OnEvent(Event& event) override;

    LLGL::Texture* Apply(
        const std::unordered_map<uint32_t, LLGL::Resource*>& resources,
        std::function<void(LLGL::CommandBuffer*)> setUniforms,
        LLGL::RenderTarget* renderTarget = nullptr,
        bool bindMatrices = false
    );

    LLGL::Texture* GetFrame() const;
    LLGL::RenderTarget* GetRenderTarget() const;

protected:
    LLGL::TextureDescriptor frameDesc;

    LLGL::Texture* frame;
    LLGL::RenderTarget* renderTarget;

    MeshPtr rect;
    LLGL::PipelineLayoutDescriptor layoutDesc;
    VertexShaderAssetPtr vertexShader;
    FragmentShaderAssetPtr fragmentShader;
    LLGL::PipelineState* rectPipeline;
};

using PostProcessingPtr = std::shared_ptr<PostProcessing>;

}

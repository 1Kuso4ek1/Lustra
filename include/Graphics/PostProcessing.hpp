#pragma once
#include <Renderer.hpp>
#include <Mesh.hpp>
#include <EventManager.hpp>
#include <AssetManager.hpp>
#include <ModelAsset.hpp>
#include <Window.hpp>
#include <ShaderAsset.hpp>

namespace lustra
{

class PostProcessing : public EventListener
{
public:
    // Make it use something like PostProcessingDescriptor?
    PostProcessing(
        const LLGL::PipelineLayoutDescriptor& layoutDesc,
        //const LLGL::GraphicsPipelineDescriptor& pipelineDesc,
        VertexShaderAssetPtr vertexShader,
        FragmentShaderAssetPtr fragmentShader,
        const LLGL::Extent2D& resolution = Renderer::Get().GetViewportResolution(),

        bool newRenderTarget = true,
        bool registerEvent = true,
        bool mipMaps = false,

        const LLGL::Format& format = LLGL::Format::RGBA16Float
    );
    ~PostProcessing();

    void OnEvent(Event& event) override;

    LLGL::Texture* Apply(
        const std::unordered_map<uint32_t, LLGL::Resource*>& resources,
        std::function<void(LLGL::CommandBuffer*)> setUniforms,
        LLGL::RenderTarget* renderTarget = nullptr,
        bool bindMatrices = false
    );

    LLGL::Texture* GetFrame();
    LLGL::RenderTarget* GetRenderTarget();

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

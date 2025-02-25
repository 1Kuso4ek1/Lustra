#include <PostProcessing.hpp>

namespace dev
{

PostProcessing::PostProcessing(
    const LLGL::PipelineLayoutDescriptor& layoutDesc,
    const LLGL::GraphicsPipelineDescriptor& pipelineDesc,
    const LLGL::Extent2D& resolution,

    bool newRenderTarget,
    bool registerEvent,
    bool mipMaps,
    
    const LLGL::Format& format
)
{
    rect = AssetManager::Get().Load<ModelAsset>("plane", true)->meshes[0];

    if(newRenderTarget)
    {
        if(registerEvent)
            EventManager::Get().AddListener(Event::Type::WindowResize, this);

        frameDesc =
        {
            .type = LLGL::TextureType::Texture2D,
            .bindFlags = LLGL::BindFlags::ColorAttachment,
            .format = format,
            .extent = { resolution.width, resolution.height, 1 },
            .mipLevels = (uint32_t)(mipMaps ? 0 : 1),
            .samples = 1
        };

        frame = Renderer::Get().CreateTexture(frameDesc);
        renderTarget = Renderer::Get().CreateRenderTarget(resolution, { frame });
    }

    rectPipeline = Renderer::Get().CreatePipelineState(layoutDesc, pipelineDesc);
}

PostProcessing::~PostProcessing()
{
    EventManager::Get().RemoveListener(Event::Type::WindowResize, this);
}

void PostProcessing::OnEvent(Event& event)
{
    if(event.GetType() == Event::Type::WindowResize)
    {
        auto resizeEvent = dynamic_cast<WindowResizeEvent*>(&event);

        LLGL::Extent2D size = resizeEvent->GetSize();

        Renderer::Get().Release(frame);
        Renderer::Get().Release(renderTarget);

        frameDesc.extent = { size.width, size.height, 1 };

        frame = Renderer::Get().CreateTexture(frameDesc);
        renderTarget = Renderer::Get().CreateRenderTarget(size, { frame });
    }
}

LLGL::Texture* PostProcessing::Apply(
    const std::unordered_map<uint32_t, LLGL::Resource*>& resources,
    std::function<void(LLGL::CommandBuffer*)> setUniforms,
    LLGL::RenderTarget* renderTarget,
    bool bindMatrices
)
{
    if(!renderTarget && !this->renderTarget)
        return frame;

    Renderer::Get().Begin();

    Renderer::Get().RenderPass(
        [&](auto commandBuffer)
        {
            rect->BindBuffers(commandBuffer, bindMatrices);

            if(frameDesc.mipLevels == 0)
                commandBuffer->GenerateMips(*frame);
        },
        resources,
        [&](auto commandBuffer)
        {
            setUniforms(commandBuffer);
            
            rect->Draw(commandBuffer);
        },
        rectPipeline,
        renderTarget ? renderTarget : this->renderTarget
    );

    Renderer::Get().End();

    Renderer::Get().Submit();

    return frame;
}

LLGL::Texture* PostProcessing::GetFrame()
{
    return frame;
}

LLGL::RenderTarget* PostProcessing::GetRenderTarget()
{
    return renderTarget;
}

}

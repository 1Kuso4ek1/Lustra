#include <PostProcessing.hpp>

namespace dev
{

PostProcessing::PostProcessing(const LLGL::PipelineLayoutDescriptor& layoutDesc,
                               LLGL::GraphicsPipelineDescriptor pipelineDesc,
                               const LLGL::Extent2D& resolution,
                               bool newRenderTarget)
{
    rect = std::make_shared<Mesh>();
    rect->CreatePlane();

    if(newRenderTarget)
    {
        LLGL::TextureDescriptor frameDesc =
        {
            .type = LLGL::TextureType::Texture2D,
            .bindFlags = LLGL::BindFlags::ColorAttachment,
            .format = LLGL::Format::RGBA16Float,
            .extent = { resolution.width, resolution.height, 1 },
            .mipLevels = 1,
            .samples = 1
        };

        frame = Renderer::Get().CreateTexture(frameDesc);
        renderTarget = Renderer::Get().CreateRenderTarget(resolution, { frame });
    }

    rectPipeline = Renderer::Get().CreatePipelineState(layoutDesc, pipelineDesc);
}

LLGL::Texture* PostProcessing::Apply(const std::unordered_map<uint32_t, LLGL::Resource*>& resources,
                                     std::function<void(LLGL::CommandBuffer*)> setUniforms,
                                     LLGL::RenderTarget* renderTarget)
{
    if(!renderTarget && !this->renderTarget)
        return frame;

    Renderer::Get().Begin();

    Renderer::Get().RenderPass(
        [&](auto commandBuffer)
        {
            rect->BindBuffers(commandBuffer, false);
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

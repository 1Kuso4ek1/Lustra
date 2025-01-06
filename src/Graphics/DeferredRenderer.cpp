#include <DeferredRenderer.hpp>

namespace dev
{

DeferredRenderer::DeferredRenderer(
    const LLGL::Extent2D& resolution,
    bool registerEvent
)
{
    if(registerEvent)
        EventManager::Get().AddListener(Event::Type::WindowResize, this);

    LLGL::TextureDescriptor colorAttachmentDesc =
    {
        .type = LLGL::TextureType::Texture2D,
        .bindFlags = LLGL::BindFlags::ColorAttachment,
        .format = LLGL::Format::RGBA16Float,
        .extent = { resolution.width, resolution.height, 1 },
        .mipLevels = 1,
        .samples = 1
    };

    LLGL::TextureDescriptor depthAttachmentDesc =
    {
        .type = LLGL::TextureType::Texture2D,
        .bindFlags = LLGL::BindFlags::DepthStencilAttachment,
        .format = LLGL::Format::D32Float,
        .extent = { resolution.width, resolution.height, 1 },
        .mipLevels = 1,
        .samples = 1
    };

    gBufferPosition = Renderer::Get().CreateTexture(colorAttachmentDesc);
    gBufferAlbedo = Renderer::Get().CreateTexture(colorAttachmentDesc);
    gBufferNormal = Renderer::Get().CreateTexture(colorAttachmentDesc);

    gBufferDepth = Renderer::Get().CreateTexture(depthAttachmentDesc);

    gBuffer = Renderer::Get().CreateRenderTarget(resolution, { gBufferPosition, gBufferAlbedo, gBufferNormal }, gBufferDepth);
    //gBufferPipeline = Renderer::Get().CreateRenderTargetPipeline(gBuffer);

    rect = std::make_shared<Mesh>();
    rect->CreatePlane();

    rectPipeline = Renderer::Get().CreatePipelineState(
        LLGL::PipelineLayoutDescriptor
        {
            .bindings =
            {
                { "gPosition", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 1 },
                { "gAlbedo", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 2 },
                { "gNormal", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 3 },
                { "lightBuffer", LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::FragmentStage, 4 },
                { "samplerState", LLGL::ResourceType::Sampler, 0, LLGL::StageFlags::FragmentStage, 1 }
            },
            .uniforms = 
            {
                { "numLights", LLGL::UniformType::Int1 },
                { "cameraPosition", LLGL::UniformType::Float3 }
            }            
        },
        LLGL::GraphicsPipelineDescriptor
        {
            .vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/screenRect.vert"),
            .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/lightingPass.frag"),
            .primitiveTopology = LLGL::PrimitiveTopology::TriangleList
        }
    );
}

DeferredRenderer::~DeferredRenderer()
{
    EventManager::Get().RemoveListener(Event::Type::WindowResize, this);
}

void DeferredRenderer::Draw(
    const std::unordered_map<uint32_t, LLGL::Resource*>& resources,
    std::function<void(LLGL::CommandBuffer*)> setUniforms,
    LLGL::RenderTarget* renderTarget
)
{
    Renderer::Get().RenderPass(
        [&](auto commandBuffer)
        {
            rect->BindBuffers(commandBuffer, false);
        },
        {
            { 0, gBufferPosition },
            { 1, gBufferAlbedo },
            { 2, gBufferNormal },
            { 3, resources.at(3) },
            { 4, AssetManager::Get().Load<TextureAsset>("")->sampler }
        },
        [&](auto commandBuffer)
        {
            setUniforms(commandBuffer);

            rect->Draw(commandBuffer);
        },
        rectPipeline,
        renderTarget
    );
}

void DeferredRenderer::OnEvent(Event& event)
{
    if(event.GetType() == Event::Type::WindowResize)
    {
        auto resizeEvent = dynamic_cast<WindowResizeEvent*>(&event);

        LLGL::Extent2D size = resizeEvent->GetSize();

        Renderer::Get().Release(gBufferPosition);
        Renderer::Get().Release(gBufferAlbedo);
        Renderer::Get().Release(gBufferNormal);
        Renderer::Get().Release(gBufferDepth);
        Renderer::Get().Release(gBuffer);

        LLGL::TextureDescriptor colorAttachmentDesc =
        {
            .type = LLGL::TextureType::Texture2D,
            .bindFlags = LLGL::BindFlags::ColorAttachment,
            .format = LLGL::Format::RGBA16Float,
            .extent = { size.width, size.height, 1 },
            .mipLevels = 1,
            .samples = 1
        };

        LLGL::TextureDescriptor depthAttachmentDesc =
        {
            .type = LLGL::TextureType::Texture2D,
            .bindFlags = LLGL::BindFlags::DepthStencilAttachment,
            .format = LLGL::Format::D32Float,
            .extent = { size.width, size.height, 1 },
            .mipLevels = 1,
            .samples = 1
        };

        gBufferPosition = Renderer::Get().CreateTexture(colorAttachmentDesc);
        gBufferAlbedo = Renderer::Get().CreateTexture(colorAttachmentDesc);
        gBufferNormal = Renderer::Get().CreateTexture(colorAttachmentDesc);

        gBufferDepth = Renderer::Get().CreateTexture(depthAttachmentDesc);

        gBuffer = Renderer::Get().CreateRenderTarget(size, { gBufferPosition, gBufferAlbedo, gBufferNormal }, gBufferDepth);
    }
}

LLGL::RenderTarget* DeferredRenderer::GetPrimaryRenderTarget()
{
    return gBuffer;
}

}
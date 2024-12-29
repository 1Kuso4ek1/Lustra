#include <Renderer.hpp>

namespace dev
{

Renderer::Renderer()
{
    try
    {
        LoadRenderSystem("OpenGL");
    }
    catch(const std::runtime_error& error)
    {
        LLGL::Log::Errorf(LLGL::Log::ColorFlags::StdError,
                          "Error: Render system loading failed: %s", error.what());
        return;
    }

    matrices = std::make_shared<Matrices>();
}

Renderer& Renderer::Get()
{
    static Renderer instance;

    return instance;
}

void Renderer::InitSwapChain(const LLGL::Extent2D& resolution, bool fullscreen, int samples)
{
    LLGL::SwapChainDescriptor swapChainDesc;

    swapChainDesc.resolution = resolution;
    swapChainDesc.fullscreen = fullscreen;
    swapChainDesc.samples = samples;

    swapChain = renderSystem->CreateSwapChain(swapChainDesc);

    SetupBuffers();
}

void Renderer::InitSwapChain(std::shared_ptr<LLGL::Surface> surface)
{   
    swapChain = renderSystem->CreateSwapChain({}, surface);

    swapChain->SetVsyncInterval(0);

    SetupBuffers();
}

void Renderer::Begin()
{
    commandBuffer->Begin();
}

void Renderer::End()
{
    commandBuffer->End();
}

void Renderer::RenderPass(std::function<void(LLGL::CommandBuffer*)> setupBuffers,
                          const std::unordered_map<uint32_t, LLGL::Resource*>& resources,
                          std::function<void(LLGL::CommandBuffer*)> draw,
                          LLGL::PipelineState* pipeline,
                          LLGL::RenderTarget* renderTarget)
{
    setupBuffers(commandBuffer);

    commandBuffer->BeginRenderPass(renderTarget ? *renderTarget : *swapChain);
    {
        //swapChain->ResizeBuffers(swapChain->GetSurface().GetContentSize());
        commandBuffer->SetViewport(renderTarget ? renderTarget->GetResolution() : swapChain->GetResolution());
        commandBuffer->Clear(LLGL::ClearFlags::ColorDepth);

        if(pipeline)
            commandBuffer->SetPipelineState(*pipeline);

        for(auto const& [key, val] : resources)
            commandBuffer->SetResource(key, *val);

        draw(commandBuffer);
    }
    commandBuffer->EndRenderPass();
}

void Renderer::Submit()
{
    commandQueue->Submit(*commandBuffer);
}

void Renderer::Present()
{
    swapChain->Present();
}

void Renderer::Unload()
{
    LLGL::RenderSystem::Unload(std::move(renderSystem));
}

void Renderer::WriteTexture(LLGL::Texture& texture, const LLGL::TextureRegion& textureRegion, const LLGL::ImageView& srcImageView)
{
    renderSystem->WriteTexture(texture, textureRegion, srcImageView);
}

LLGL::Buffer* Renderer::CreateBuffer(const LLGL::BufferDescriptor& bufferDesc, const void* initialData)
{
    return renderSystem->CreateBuffer(bufferDesc, initialData);
}

LLGL::Shader* Renderer::CreateShader(const LLGL::ShaderType& type, const std::filesystem::path& path, const std::vector<LLGL::VertexAttribute>& attributes)
{
    LLGL::ShaderDescriptor shaderDesc = { type, path.c_str() };

    if(type == LLGL::ShaderType::Vertex)
        shaderDesc.vertex.inputAttribs = attributes.empty() ? defaultVertexFormat.attributes : attributes;

    auto shader = renderSystem->CreateShader(shaderDesc);

    if(const LLGL::Report* report = shader->GetReport())
    {
        if(report->HasErrors())
            LLGL::Log::Errorf(LLGL::Log::ColorFlags::StdError, 
                              "Shader compile errors:\n\t%s\n%s",
                                      path.filename().string().c_str(), report->GetText());
        else
            LLGL::Log::Errorf(LLGL::Log::ColorFlags::StdWarning, 
                              "Shader compile warnings:\n\t%s\n%s",
                                      path.filename().string().c_str(), report->GetText());
    }

    return shader;
}

LLGL::Texture* Renderer::CreateTexture(const LLGL::TextureDescriptor& textureDesc, const LLGL::ImageView* initialImage)
{
    return renderSystem->CreateTexture(textureDesc, initialImage);
}

LLGL::Sampler* Renderer::CreateSampler(const LLGL::SamplerDescriptor& samplerDesc)
{
    return renderSystem->CreateSampler(samplerDesc);
}

LLGL::RenderTarget* Renderer::CreateRenderTarget(const LLGL::Extent2D& resolution, const std::vector<LLGL::Texture*>& colorAttachments, LLGL::Texture* depthTexture)
{
    LLGL::RenderTargetDescriptor renderTargetDesc;
    renderTargetDesc.resolution = resolution;

    const auto max = LLGL_MAX_NUM_COLOR_ATTACHMENTS;

    for(int i = 0; i < (colorAttachments.size() > max ? max : colorAttachments.size()); i++)
        renderTargetDesc.colorAttachments[i] = colorAttachments[i];

    if(depthTexture)
        renderTargetDesc.depthStencilAttachment = depthTexture;

    return renderSystem->CreateRenderTarget(renderTargetDesc);
}

LLGL::PipelineState* Renderer::CreatePipelineState(LLGL::Shader* vertexShader, LLGL::Shader* fragmentShader)
{
    LLGL::PipelineLayoutDescriptor layoutDesc;

    layoutDesc.bindings =
    {
        { "matrices", LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage, 1 },
        { "albedo", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 2 },
        { "samplerState", LLGL::ResourceType::Sampler, 0, LLGL::StageFlags::FragmentStage, 2 }
    };

    LLGL::PipelineLayout* pipelineLayout = renderSystem->CreatePipelineLayout(layoutDesc);

    LLGL::BlendTargetDescriptor blendTargetDesc;
    blendTargetDesc.blendEnabled = true;

    LLGL::BlendDescriptor blendDesc;
    blendDesc.independentBlendEnabled = true;
    blendDesc.targets[0] = blendTargetDesc;

    LLGL::GraphicsPipelineDescriptor pipelineStateDesc;
    pipelineStateDesc.vertexShader = vertexShader;
    pipelineStateDesc.fragmentShader = fragmentShader;
    pipelineStateDesc.pipelineLayout = pipelineLayout;
    pipelineStateDesc.renderPass = swapChain->GetRenderPass();
    pipelineStateDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleList;

    pipelineStateDesc.depth.testEnabled = true;
    pipelineStateDesc.depth.writeEnabled = true;

    pipelineStateDesc.blend = blendDesc;

    pipelineStateDesc.rasterizer.cullMode = LLGL::CullMode::Back;
    pipelineStateDesc.rasterizer.frontCCW = true;
    pipelineStateDesc.rasterizer.multiSampleEnabled = (swapChain->GetSamples() > 1);

    return renderSystem->CreatePipelineState(pipelineStateDesc);
}

LLGL::PipelineState* Renderer::CreatePipelineState(const LLGL::PipelineLayoutDescriptor& layoutDesc,
                                                   LLGL::GraphicsPipelineDescriptor pipelineDesc)
{
    LLGL::PipelineLayout* pipelineLayout = renderSystem->CreatePipelineLayout(layoutDesc);

    pipelineDesc.pipelineLayout = pipelineLayout;

    return renderSystem->CreatePipelineState(pipelineDesc);
}

LLGL::PipelineState* Renderer::CreateRenderTargetPipeline(LLGL::RenderTarget* renderTarget)
{
    LLGL::GraphicsPipelineDescriptor pipelineStateDesc;
    pipelineStateDesc.renderPass = renderTarget->GetRenderPass();
    pipelineStateDesc.viewports  = { renderTarget->GetResolution() };
    pipelineStateDesc.rasterizer.multiSampleEnabled = false;

    return renderSystem->CreatePipelineState(pipelineStateDesc);
}

LLGL::SwapChain* Renderer::GetSwapChain() const
{
    return swapChain;
}

LLGL::Window* Renderer::GetWindow() const
{
    return swapChain ? &LLGL::CastTo<LLGL::Window>(swapChain->GetSurface()) : nullptr;
}

LLGL::VertexFormat Renderer::GetDefaultVertexFormat() const
{
    return defaultVertexFormat;
}

LLGL::Buffer* Renderer::GetMatricesBuffer() const
{
    return matricesBuffer;
}

std::shared_ptr<Matrices> Renderer::GetMatrices() const
{
    return matrices;
}

bool Renderer::IsInit()
{
    return renderSystem != nullptr;// && swapChain != nullptr;
}

void Renderer::LoadRenderSystem(const LLGL::RenderSystemDescriptor& desc)
{
    LLGL::Report report;

    renderSystem = LLGL::RenderSystem::Load(desc, &report);
    if(renderSystem == nullptr)
        throw std::runtime_error(report.GetText());

    commandQueue = renderSystem->GetCommandQueue();
}

void Renderer::SetupDefaultVertexFormat()
{
    defaultVertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });
    defaultVertexFormat.AppendAttribute({ "normal", LLGL::Format::RGB32Float });
    defaultVertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float });
}

void Renderer::SetupCommandBuffer()
{
    commandBuffer = renderSystem->CreateCommandBuffer();
    commandQueue = renderSystem->GetCommandQueue();
}

void Renderer::CreateMatricesBuffer()
{
    LLGL::BufferDescriptor bufferDesc = LLGL::ConstantBufferDesc(sizeof(Matrices::Binding));

    matricesBuffer = renderSystem->CreateBuffer(bufferDesc);
}

void Renderer::SetupBuffers()
{
    SetupDefaultVertexFormat();
    SetupCommandBuffer();
    CreateMatricesBuffer();
}

}

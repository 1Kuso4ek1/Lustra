#include <Renderer.hpp>

namespace lustra
{

void Renderer::Init()
{
    if(renderSystem)
    {
        LLGL::Log::Printf(
            LLGL::Log::ColorFlags::StdWarning,
            "Renderer already initialized\n"
        );

        return;
    }

    try
    {
        LoadRenderSystem("OpenGL");
    }
    catch(const std::runtime_error& error)
    {
        LLGL::Log::Errorf(
            LLGL::Log::ColorFlags::StdError,
            "Error: Render system loading failed: %s", error.what()
        );
        return;
    }

    const LLGL::RendererInfo& info = renderSystem->GetRendererInfo();

    LLGL::Log::Printf(
        LLGL::Log::ColorFlags::Bold | LLGL::Log::ColorFlags::Blue,
        "Render system:\n"
    );

    LLGL::Log::Printf(
        LLGL::Log::ColorFlags::Blue,
        "*  Graphics API:       %s\n"
        "*  Device:             %s\n"
        "*  Vendor:             %s\n"
        "*  Shading language:   %s\n"
        "\n",
        info.rendererName.c_str(),
        info.deviceName.c_str(),
        info.vendorName.c_str(),
        info.shadingLanguageName.c_str()
    );

    if(!matrices)
        matrices = std::make_shared<Matrices>();
}

void Renderer::InitSwapChain(const LLGL::Extent2D& resolution, const bool fullscreen, const int samples)
{
    LLGL::SwapChainDescriptor swapChainDesc;

    swapChainDesc.resolution = resolution;
    swapChainDesc.fullscreen = fullscreen;
    swapChainDesc.samples = samples;

    swapChain = renderSystem->CreateSwapChain(swapChainDesc);

    viewportResolution = resolution;

    SetupBuffers();
}

void Renderer::InitSwapChain(const std::shared_ptr<LLGL::Surface>& surface)
{
    swapChain = renderSystem->CreateSwapChain({}, surface);

    viewportResolution = swapChain->GetResolution();

    SetupBuffers();
}

void Renderer::Begin()
{
    renderPassCounter = 0;

    commandBuffer->Begin();
}

void Renderer::End() const
{
    commandBuffer->End();
}

void Renderer::RenderPass(
    const std::function<void(LLGL::CommandBuffer*)>& setupBuffers,
    const std::unordered_map<uint32_t, LLGL::Resource*>& resources,
    const std::function<void(LLGL::CommandBuffer*)>& draw,
    LLGL::PipelineState* pipeline,
    LLGL::RenderTarget* renderTarget
)
{
    setupBuffers(commandBuffer);

    commandBuffer->BeginRenderPass(renderTarget ? *renderTarget : *swapChain);
    {
        swapChain->ResizeBuffers(swapChain->GetSurface().GetContentSize());

        if(pipeline)
        {
            commandBuffer->SetViewport(renderTarget ? renderTarget->GetResolution() : swapChain->GetResolution());

            if(renderPassCounter == 0)
                commandBuffer->Clear(LLGL::ClearFlags::ColorDepth);

            commandBuffer->SetPipelineState(*pipeline);

            renderPassCounter++;
        }

        for(auto const& [key, val] : resources)
            commandBuffer->SetResource(key, *val);

        draw(commandBuffer);
    }
    commandBuffer->EndRenderPass();
}

void Renderer::Submit() const
{
    commandQueue->Submit(*commandBuffer);
}

void Renderer::Present() const
{
    swapChain->Present();
}

void Renderer::ClearRenderTarget(LLGL::RenderTarget* renderTarget, const bool begin)
{
    if(begin)
        Begin();

    RenderPass(
        [](auto){}, {},
        [](auto commandBuffer)
        {
            commandBuffer->Clear(LLGL::ClearFlags::ColorDepth);
        },
        nullptr,
        renderTarget
    );

    if(begin)
    {
        End();

        Submit();
    }
}

void Renderer::GenerateMips(LLGL::Texture* texture)
{
    Begin();

    RenderPass(
        [&](auto)
        {
            commandBuffer->GenerateMips(*texture);
        }, {}, [&](auto) {},
        nullptr
    );

    End();

    Submit();
}

void Renderer::Unload()
{
    renderSystem->Release(*matricesBuffer);
    renderSystem->Release(*commandBuffer);
    renderSystem->Release(*swapChain);

    defaultVertexFormat = LLGL::VertexFormat();

    LLGL::RenderSystem::Unload(std::move(renderSystem));
}

void Renderer::WriteTexture(LLGL::Texture& texture, const LLGL::TextureRegion& textureRegion, const LLGL::ImageView& srcImageView) const
{
    renderSystem->WriteTexture(texture, textureRegion, srcImageView);
}

void Renderer::SetViewportResolution(const LLGL::Extent2D& resolution)
{
    viewportResolution = resolution;
}

LLGL::Extent2D Renderer::GetViewportResolution() const
{
    return viewportResolution;
}

LLGL::Buffer* Renderer::CreateBuffer(const LLGL::BufferDescriptor& bufferDesc, const void* initialData) const
{
    return renderSystem->CreateBuffer(bufferDesc, initialData);
}

LLGL::Buffer* Renderer::CreateBuffer(const std::string& name, const LLGL::BufferDescriptor& bufferDesc, const void* initialData)
{
    const auto it = globalBuffers.find(name);

    if(it != globalBuffers.end())
        return it->second;

    const auto buffer = renderSystem->CreateBuffer(bufferDesc, initialData);

    globalBuffers[name] = buffer;

    return buffer;
}

LLGL::Shader* Renderer::CreateShader(const LLGL::ShaderType& type, const std::filesystem::path& path, const std::vector<LLGL::VertexAttribute>& attributes) const
{
    const auto strPath = path.string();
    LLGL::ShaderDescriptor shaderDesc{ type, strPath.c_str() };

    if(type == LLGL::ShaderType::Vertex)
        shaderDesc.vertex.inputAttribs = attributes.empty() ? defaultVertexFormat.attributes : attributes;

    const auto shader = renderSystem->CreateShader(shaderDesc);

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

LLGL::Texture* Renderer::CreateTexture(const LLGL::TextureDescriptor& textureDesc, const LLGL::ImageView* initialImage) const
{
    return renderSystem->CreateTexture(textureDesc, initialImage);
}

LLGL::Sampler* Renderer::CreateSampler(const LLGL::SamplerDescriptor& samplerDesc) const
{
    return renderSystem->CreateSampler(samplerDesc);
}

LLGL::RenderTarget* Renderer::CreateRenderTarget(const LLGL::Extent2D& resolution, const std::vector<LLGL::AttachmentDescriptor>& colorAttachments, LLGL::Texture* depthTexture) const
{
    LLGL::RenderTargetDescriptor renderTargetDesc;
    renderTargetDesc.resolution = resolution;

    constexpr auto max = LLGL_MAX_NUM_COLOR_ATTACHMENTS;

    for(int i = 0; i < (colorAttachments.size() > max ? max : colorAttachments.size()); i++)
        renderTargetDesc.colorAttachments[i] = colorAttachments[i];

    if(depthTexture)
        renderTargetDesc.depthStencilAttachment = depthTexture;

    return renderSystem->CreateRenderTarget(renderTargetDesc);
}

LLGL::PipelineState* Renderer::CreatePipelineState(LLGL::Shader* vertexShader, LLGL::Shader* fragmentShader)
{
    uint64_t key = reinterpret_cast<uint64_t>(vertexShader) ^ reinterpret_cast<uint64_t>(fragmentShader);

    auto it = pipelineCache.find(key);
    if(it != pipelineCache.end())
        return it->second;

    LLGL::PipelineLayoutDescriptor layoutDesc;

    layoutDesc.bindings =
    {
        { "matrices", LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage, 1 },
        { "albedoTexture", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 2 },
        { "normalTexture", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 3 },
        { "metallicTexture", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 4 },
        { "roughnessTexture", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 5 },
        { "aoTexture", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 6 },
        { "emissionTexture", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 7 },
        { "samplerState", LLGL::ResourceType::Sampler, 0, LLGL::StageFlags::FragmentStage, 2 }
    };

    layoutDesc.uniforms =
    {
        { "albedoType", LLGL::UniformType::Int1 },
        { "albedoValue", LLGL::UniformType::Float4 },
        { "normalType", LLGL::UniformType::Int1 },
        { "metallicType", LLGL::UniformType::Int1 },
        { "metallicValue", LLGL::UniformType::Float1 },
        { "roughnessType", LLGL::UniformType::Int1 },
        { "roughnessValue", LLGL::UniformType::Float1 },
        { "aoType", LLGL::UniformType::Int1 },
        { "emissionType", LLGL::UniformType::Int1 },
        { "emissionValue", LLGL::UniformType::Float3 },
        { "emissionStrength", LLGL::UniformType::Float1 },
        { "uvScale", LLGL::UniformType::Float2 },
        { "uvOffset", LLGL::UniformType::Float2 },
        { "time", LLGL::UniformType::Float1 }
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

    pipelineStateDesc.depth.testEnabled = true;
    pipelineStateDesc.depth.writeEnabled = true;

    pipelineStateDesc.blend = blendDesc;

    pipelineStateDesc.rasterizer.cullMode = LLGL::CullMode::Back;
    pipelineStateDesc.rasterizer.frontCCW = true;
    pipelineStateDesc.rasterizer.multiSampleEnabled = (swapChain->GetSamples() > 1);

    pipelineCache[key] = renderSystem->CreatePipelineState(pipelineStateDesc);

    return pipelineCache[key];
}

LLGL::PipelineState* Renderer::CreatePipelineState(
    const LLGL::PipelineLayoutDescriptor& layoutDesc,
    LLGL::GraphicsPipelineDescriptor pipelineDesc
) const
{
    const auto pipelineLayout = renderSystem->CreatePipelineLayout(layoutDesc);

    pipelineDesc.pipelineLayout = pipelineLayout;

    return renderSystem->CreatePipelineState(pipelineDesc);
}

LLGL::PipelineState* Renderer::CreateRenderTargetPipeline(const LLGL::RenderTarget* renderTarget) const
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

bool Renderer::IsInit() const
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
    const auto bufferDesc = LLGL::ConstantBufferDesc(sizeof(Matrices::Binding));

    matricesBuffer = renderSystem->CreateBuffer(bufferDesc);
}

void Renderer::SetupBuffers()
{
    SetupDefaultVertexFormat();
    SetupCommandBuffer();
    CreateMatricesBuffer();
}

}

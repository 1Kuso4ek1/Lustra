#include <Renderer.hpp>

Renderer::Renderer()
{
    try
    {
        LoadRenderSystem("OpenGL");
    }
    catch(const std::runtime_error& error)
    {
        LLGL::Log::Errorf("Error: Render system loading failed: %s", error.what());
        return;
    }

    SetupDefaultVertexFormat();
    SetupDefaultPipeline();
    SetupCommandBuffer();

    matrices = std::make_shared<Matrices>();
}

Renderer& Renderer::Get()
{
    static std::unique_ptr<Renderer> instance = std::unique_ptr<Renderer>(new Renderer());

    return *instance;
}

void Renderer::InitSwapChain(const LLGL::Extent2D& resolution, bool fullscreen, int samples)
{
    LLGL::SwapChainDescriptor swapChainDesc;

    swapChainDesc.resolution = resolution;
    swapChainDesc.fullscreen = fullscreen;
    swapChainDesc.samples = samples;

    swapChain = renderSystem->CreateSwapChain(swapChainDesc);
}

void Renderer::RenderPass(std::function<void(LLGL::CommandBuffer*)> setupBuffers,
                          std::unordered_map<uint32_t, LLGL::Resource*> resources,
                          std::function<void(LLGL::CommandBuffer*)> draw,
                          LLGL::PipelineState* pipeline)
{
    commandBuffer->Begin();
    {
        setupBuffers(commandBuffer);

        commandBuffer->BeginRenderPass(*swapChain);
        {
            commandBuffer->SetViewport(swapChain->GetResolution());
            commandBuffer->Clear(LLGL::ClearFlags::ColorDepth, { 0.0, 0.0, 0.0, 0.0 });

            commandBuffer->SetPipelineState(pipeline ? *pipeline : *defaultPipeline);

            for(auto const& [key, val] : resources)
                commandBuffer->SetResource(key, *val);

            draw(commandBuffer);
        }
        commandBuffer->EndRenderPass();
    }
    commandBuffer->End();
}

void Renderer::Present()
{
    swapChain->Present();
}

LLGL::Buffer* Renderer::CreateBuffer(const LLGL::BufferDescriptor& bufferDesc, const void* data)
{
    return renderSystem->CreateBuffer(bufferDesc, data);
}

LLGL::Window* Renderer::GetWindow() const
{
    return swapChain ? &LLGL::CastTo<LLGL::Window>(swapChain->GetSurface()) : nullptr;
}

LLGL::VertexFormat Renderer::GetDefaultVertexFormat() const
{
    return defaultVertexFormat;
}

LLGL::PipelineState* Renderer::GetDefaultPipeline() const
{
    return defaultPipeline;
}

bool Renderer::IsInit()
{
    return renderSystem != nullptr;
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

void Renderer::SetupDefaultPipeline()
{
    LLGL::PipelineLayoutDescriptor layoutDesc;
    layoutDesc.bindings =
    {
        { "matrices", LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage, 0 },
        { "albedo", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 1 },
        { "samplerState", LLGL::ResourceType::Sampler, 0, LLGL::StageFlags::FragmentStage, 1 }
    };

    LLGL::PipelineLayout* pipelineLayout = renderSystem->CreatePipelineLayout(layoutDesc);

    LLGL::BlendTargetDescriptor blendTargetDesc;
    blendTargetDesc.blendEnabled = true;

    LLGL::BlendDescriptor blendDesc;
    blendDesc.independentBlendEnabled = true;
    blendDesc.targets[0] = blendTargetDesc;

    LLGL::GraphicsPipelineDescriptor pipelineStateDesc;
    pipelineStateDesc.pipelineLayout = pipelineLayout;
    pipelineStateDesc.renderPass = swapChain->GetRenderPass();
    pipelineStateDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleList;

    pipelineStateDesc.depth.testEnabled = true;
    pipelineStateDesc.depth.writeEnabled = true;

    pipelineStateDesc.blend = blendDesc;

    pipelineStateDesc.rasterizer.cullMode = LLGL::CullMode::Front;
    pipelineStateDesc.rasterizer.multiSampleEnabled = (swapChain->GetSamples() > 1);

    defaultPipeline = renderSystem->CreatePipelineState(pipelineStateDesc);
}

void Renderer::SetupCommandBuffer()
{
    commandBuffer = renderSystem->CreateCommandBuffer(LLGL::CommandBufferFlags::ImmediateSubmit);
}

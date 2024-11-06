#include <Application.hpp>

Application::Application()
{
    LLGL::Log::RegisterCallbackStd();

    try
    {
        LoadRenderSystem("OpenGL");
    }
    catch(const std::runtime_error& error)
    {
        LLGL::Log::Errorf("Error: Render system loading failed: %s", error.what());
        return;
    }

    InitSwapChain({ 800, 600 });

    SetupVertexFormat();

    CreateVertexBuffer();

    LoadShaders();

    CreatePipeline();

    CreateCommandBuffer();
}

Application::~Application() {}

void Application::Run()
{
    if(swapChain == nullptr)
    {
        LLGL::Log::Errorf("Error: Trying to run app with no swap chain\n");
        return;
    }

    LLGL::Window& window = LLGL::CastTo<LLGL::Window>(swapChain->GetSurface());

    window.SetTitle("LLGLTest");
    window.Show();

    while(window.ProcessEvents() && !window.HasQuit())
    {
        commandBuffer->Begin();
        
        commandBuffer->SetVertexBuffer(*vertexBuffer);
        
        commandBuffer->BeginRenderPass(*swapChain);

        commandBuffer->SetViewport(swapChain->GetResolution());

        commandBuffer->Clear(LLGL::ClearFlags::Color, { 0.1, 0.1, 0.1, 1.0 });

        commandBuffer->SetPipelineState(*pipeline);

        commandBuffer->Draw(3, 0);

        commandBuffer->EndRenderPass();

        commandBuffer->End();

        swapChain->Present();
    }
}

void Application::LoadRenderSystem(const LLGL::RenderSystemDescriptor& desc)
{
    LLGL::Report report;

    renderSystem = LLGL::RenderSystem::Load(desc, &report);
    if(renderSystem == nullptr)
        throw std::runtime_error(report.GetText());
}

void Application::InitSwapChain(const LLGL::Extent2D& resolution, bool fullscreen, int samples)
{
    swapChainDescriptor.resolution = resolution;
    swapChainDescriptor.fullscreen = fullscreen;
    swapChainDescriptor.samples = samples;

    swapChain = renderSystem->CreateSwapChain(swapChainDescriptor);
}

void Application::SetupVertexFormat()
{
    vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float });
    vertexFormat.AppendAttribute({ "color",    LLGL::Format::RGBA8UNorm });
}

void Application::CreateVertexBuffer()
{
    vertexBufferDescriptor.size = sizeof(vertices);
    vertexBufferDescriptor.bindFlags = LLGL::BindFlags::VertexBuffer;
    vertexBufferDescriptor.vertexAttribs = vertexFormat.attributes;

    vertexBuffer = renderSystem->CreateBuffer(vertexBufferDescriptor, vertices);
}

void Application::LoadShaders()
{
    vertexShaderDescriptor = { LLGL::ShaderType::Vertex, "../shaders/vertex.vert" };
    fragmentShaderDescriptor = { LLGL::ShaderType::Fragment, "../shaders/fragment.frag" };

    vertexShaderDescriptor.vertex.inputAttribs = vertexFormat.attributes;

    vertexShader = renderSystem->CreateShader(vertexShaderDescriptor);
    fragmentShader = renderSystem->CreateShader(fragmentShaderDescriptor);

    for(LLGL::Shader* shader : { vertexShader, fragmentShader })
    {
        if(const LLGL::Report* report = shader->GetReport())
        {
            if(report->HasErrors())
                LLGL::Log::Errorf("Shader compile errors:\n%s", report->GetText());
            else
                LLGL::Log::Printf("Shader compile warnings:\n%s", report->GetText());
        }
    }
}

void Application::CreatePipeline()
{
    pipelineDescriptor.vertexShader = vertexShader;
    pipelineDescriptor.fragmentShader = fragmentShader;
    pipelineDescriptor.rasterizer.multiSampleEnabled = (swapChainDescriptor.samples > 1);

    pipeline = renderSystem->CreatePipelineState(pipelineDescriptor);
}

void Application::CreateCommandBuffer()
{
    commandBuffer = renderSystem->CreateCommandBuffer(LLGL::CommandBufferFlags::ImmediateSubmit);
}

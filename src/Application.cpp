#include <Application.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

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

    InitSwapChain({ 800, 800 });

    SetupVertexFormat();

    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateMatricesBuffer();

    LoadShaders();

    CreatePipeline();

    CreateCommandBuffer();

    LoadTextures();
    
    matrices = std::make_shared<Matrices>();

    matrices->Translate({ 0.f, 0.f, -5.f });
    matrices->Scale({ 1.f, 1.f, 1.f });
    matrices->Rotate(glm::radians(45.f), { 0.3f, 0.3f, 0.3f });

    matrices->GetProjection() = glm::perspective(glm::radians(90.0f), 800.0f / 800.0f, 0.1f, 100.0f);
    matrices->GetView() = glm::lookAt(glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
}

Application::~Application() {}

void Application::Run()
{
    if(renderSystem == nullptr)
    {
        LLGL::Log::Errorf("Error: Trying to run app with no render system\n");
        return;
    }

    LLGL::Window& window = LLGL::CastTo<LLGL::Window>(swapChain->GetSurface());

    window.SetTitle("LLGLTest");
    window.Show();

    while(window.ProcessEvents() && !window.HasQuit())
    {
        auto matricesBinding = matrices->GetBinding();

        commandBuffer->Begin();
        {
            commandBuffer->SetVertexBuffer(*vertexBuffer);
            commandBuffer->SetIndexBuffer(*indexBuffer);
            commandBuffer->UpdateBuffer(*matricesBuffer, 0, &matricesBinding, sizeof(Matrices::MatricesBinding));
            
            commandBuffer->BeginRenderPass(*swapChain);
            {
                commandBuffer->SetViewport(swapChain->GetResolution());
                commandBuffer->Clear(LLGL::ClearFlags::ColorDepth, { 0.1, 0.1, 0.1, 1.0 });

                commandBuffer->SetPipelineState(*pipeline);

                commandBuffer->SetResource(0, *matricesBuffer);
                commandBuffer->SetResource(1, *texture);
                commandBuffer->SetResource(2, *sampler);

                commandBuffer->DrawIndexed(sizeof(indices) / sizeof(uint32_t), 0);
            }
            commandBuffer->EndRenderPass();
        }
        commandBuffer->End();

        commandQueue->Submit(*commandBuffer);

        swapChain->Present();
    }
}

void Application::LoadRenderSystem(const LLGL::RenderSystemDescriptor& desc)
{
    LLGL::Report report;

    renderSystem = LLGL::RenderSystem::Load(desc, &report);
    if(renderSystem == nullptr)
        throw std::runtime_error(report.GetText());

    commandQueue = renderSystem->GetCommandQueue();
}

void Application::InitSwapChain(const LLGL::Extent2D& resolution, bool fullscreen, int samples)
{
    LLGL::SwapChainDescriptor swapChainDesc;

    swapChainDesc.resolution = resolution;
    swapChainDesc.fullscreen = fullscreen;
    swapChainDesc.samples = samples;

    swapChain = renderSystem->CreateSwapChain(swapChainDesc);
}

void Application::SetupVertexFormat()
{
    vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });
    vertexFormat.AppendAttribute({ "normal", LLGL::Format::RGB32Float });
    vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float });
}

void Application::CreateVertexBuffer()
{
    LLGL::BufferDescriptor bufferDesc = LLGL::VertexBufferDesc(sizeof(vertices), vertexFormat);

    vertexBuffer = renderSystem->CreateBuffer(bufferDesc, vertices);
}

void Application::CreateIndexBuffer()
{
    LLGL::BufferDescriptor bufferDesc = LLGL::IndexBufferDesc(sizeof(indices), LLGL::Format::R32UInt);

    indexBuffer = renderSystem->CreateBuffer(bufferDesc, indices);
}

void Application::CreateMatricesBuffer()
{
    LLGL::BufferDescriptor bufferDesc = LLGL::ConstantBufferDesc(sizeof(Matrices::MatricesBinding));

    matricesBuffer = renderSystem->CreateBuffer(bufferDesc);
}

void Application::LoadShaders()
{
    LLGL::ShaderDescriptor vertexShaderDesc = { LLGL::ShaderType::Vertex, "../shaders/vertex.vert" };
    LLGL::ShaderDescriptor fragmentShaderDesc = { LLGL::ShaderType::Fragment, "../shaders/fragment.frag" };

    vertexShaderDesc.vertex.inputAttribs = vertexFormat.attributes;

    vertexShader = renderSystem->CreateShader(vertexShaderDesc);
    fragmentShader = renderSystem->CreateShader(fragmentShaderDesc);

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
    pipelineStateDesc.vertexShader = vertexShader;
    pipelineStateDesc.fragmentShader = fragmentShader;
    pipelineStateDesc.pipelineLayout = pipelineLayout;
    pipelineStateDesc.renderPass = swapChain->GetRenderPass();
    pipelineStateDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleList;
    
    pipelineStateDesc.depth.testEnabled = true;
    pipelineStateDesc.depth.writeEnabled = true;

    pipelineStateDesc.blend = blendDesc;
    
    pipelineStateDesc.rasterizer.cullMode = LLGL::CullMode::Front;
    pipelineStateDesc.rasterizer.multiSampleEnabled = (swapChain->GetSamples() > 1);

    pipeline = renderSystem->CreatePipelineState(pipelineStateDesc);
}

void Application::CreateCommandBuffer()
{
    commandBuffer = renderSystem->CreateCommandBuffer(LLGL::CommandBufferFlags::ImmediateSubmit);
}

void Application::LoadTextures()
{
    LLGL::ImageView imageView;
    imageView.format = LLGL::ImageFormat::RGBA;

    int width, height, channels;
    unsigned char* data = stbi_load("../textures/tex.jpg", &width, &height, &channels, 4);

    if(data)
    {
        imageView.data = data;
        imageView.dataSize = width * height * 4 * 8;
        imageView.dataType = LLGL::DataType::UInt8;

        LLGL::TextureDescriptor textureDesc;
        textureDesc.type = LLGL::TextureType::Texture2D;
        textureDesc.format = LLGL::Format::RGBA8UNorm;
        textureDesc.extent = { (uint32_t)width, (uint32_t)height, 1 };
        textureDesc.miscFlags = LLGL::MiscFlags::GenerateMips;

        texture = renderSystem->CreateTexture(textureDesc, &imageView);

        LLGL::SamplerDescriptor samplerDesc;
        samplerDesc.maxAnisotropy = 8;

        sampler = renderSystem->CreateSampler(samplerDesc);
    }
    else
        LLGL::Log::Errorf("Failed to load texture");

    stbi_image_free(data);
}

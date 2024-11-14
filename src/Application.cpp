#include "Renderer.hpp"
#include <Application.hpp>
#include <LLGL/RenderSystem.h>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Application::Application()
{
    LLGL::Log::RegisterCallbackStd();

    /*try
    {
        LoadRenderSystem("OpenGL");
    }
    catch(const std::runtime_error& error)
    {
        LLGL::Log::Errorf("Error: Render system loading failed: %s", error.what());
        return;
    }*/

    Renderer::Get().InitSwapChain({ 800, 800 });

    //SetupVertexFormat();

    LoadShaders();

    /*CreatePipeline();

    CreateCommandBuffer();*/

    LoadTextures();

    mesh = std::make_unique<Mesh>();
    mesh->CreateCube();
    
    pipeline = Renderer::Get().CreatePipelineState(vertexShader, fragmentShader);
    matrices = Renderer::Get().GetMatrices();

    matrices->Translate({ 0.f, 0.f, -5.f });
    matrices->Scale({ 1.f, 1.f, 1.f });
    matrices->Rotate(glm::radians(45.f), { 0.3f, 0.3f, 0.3f });

    matrices->GetProjection() = glm::perspective(glm::radians(90.0f), 800.0f / 800.0f, 0.1f, 100.0f);
    matrices->GetView() = glm::lookAt(glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
}

Application::~Application() {}

void Application::Run()
{
    if(!Renderer::Get().IsInit())
    {
        LLGL::Log::Errorf("Error: Renderer is not initialized\n");
        return;
    }

    auto window = Renderer::Get().GetWindow();

    window->SetTitle("LLGLTest");
    window->Show();

    while(window->ProcessEvents() && !window->HasQuit())
    {
        Renderer::Get().RenderPass(
        [&](auto commandBuffer) { mesh->BindBuffers(commandBuffer); },
        { { 0, Renderer::Get().GetMatricesBuffer() },
                    { 1, texture },
                    { 2, sampler } },
        [&](auto commandBuffer) { mesh->Draw(commandBuffer); },
        pipeline);

        Renderer::Get().Present();
    }
}

void Application::LoadShaders()
{
    LLGL::ShaderDescriptor vertexShaderDesc = { LLGL::ShaderType::Vertex, "../shaders/vertex.vert" };
    LLGL::ShaderDescriptor fragmentShaderDesc = { LLGL::ShaderType::Fragment, "../shaders/fragment.frag" };

    vertexShaderDesc.vertex.inputAttribs = Renderer::Get().GetDefaultVertexFormat().attributes;

    vertexShader = Renderer::Get().CreateShader(vertexShaderDesc);
    fragmentShader = Renderer::Get().CreateShader(fragmentShaderDesc);

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

        texture = Renderer::Get().CreateTexture(textureDesc, &imageView);

        LLGL::SamplerDescriptor samplerDesc;
        samplerDesc.maxAnisotropy = 8;

        sampler = Renderer::Get().CreateSampler(samplerDesc);
    }
    else
        LLGL::Log::Errorf("Failed to load texture");

    stbi_image_free(data);
}

#include <CubeApp.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

CubeApp::CubeApp()
{
    LLGL::Log::RegisterCallbackStd();

    Renderer::Get().InitSwapChain({ 800, 800 });

    LoadShaders();

    LoadTextures();

    mesh = std::make_unique<Mesh>();
    mesh->CreateCube();
    
    pipeline = Renderer::Get().CreatePipelineState(vertexShader, fragmentShader);
    matrices = Renderer::Get().GetMatrices();

    matrices->GetProjection() = glm::perspective(glm::radians(90.0f), 800.0f / 800.0f, 0.1f, 100.0f);
    matrices->GetView() = glm::lookAt(glm::vec3(0.f, 0.f, 5.f), { 0, 0, 0.f }, glm::vec3(0.f, 1.f, 0.f));
}

void CubeApp::Run()
{
    if(!Renderer::Get().IsInit())
    {
        LLGL::Log::Errorf("Error: Renderer is not initialized\n");
        return;
    }

    auto window = Renderer::Get().GetWindow();

    window->SetTitle("LLGLTest");
    window->Show();

    LLGL::Input input(*window);

    while(window->ProcessEvents() && !window->HasQuit())
    {
        Renderer::Get().RenderPass(
        [&](auto commandBuffer) { mesh->BindBuffers(commandBuffer); },
        {
            { 0, Renderer::Get().GetMatricesBuffer() },
            { 1, texture },
            { 2, sampler }
        },
        [&](auto commandBuffer) { mesh->Draw(commandBuffer); },
        pipeline);

        Renderer::Get().Present();

        if(input.KeyPressed(LLGL::Key::Q))
            degrees -= 0.1f;
        else if(input.KeyPressed(LLGL::Key::E))
            degrees += 0.1f;

        matrices->Rotate(glm::radians(degrees), { 0.0f, 1.0f, 0.0f });
    }
}

void CubeApp::LoadShaders()
{
    vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/vertex.vert");
    fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/fragment.frag");
}

void CubeApp::LoadTextures()
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

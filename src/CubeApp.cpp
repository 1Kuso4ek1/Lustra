#include "Mouse.hpp"
#include <CubeApp.hpp>

CubeApp::CubeApp()
{
    LLGL::Log::RegisterCallbackStd();

    window = std::make_shared<dev::Window>(LLGL::Extent2D{ 800, 800 }, "LLGLTest");

    dev::Renderer::Get().InitSwapChain({ 800, 800 }, window);

    LoadShaders();
    LoadTextures();

    (mesh = std::make_unique<dev::Mesh>())->CreateCube();
    
    pipeline = dev::Renderer::Get().CreatePipelineState(vertexShader, fragmentShader);
    matrices = dev::Renderer::Get().GetMatrices();

    matrices->GetProjection() = glm::perspective(glm::radians(90.0f), 800.0f / 800.0f, 0.1f, 100.0f);
    matrices->GetView() = glm::lookAt(glm::vec3(0.f, 0.f, 5.f), { 0, 0, 0.f }, glm::vec3(0.f, 1.f, 0.f));
}

void CubeApp::Run()
{
    if(!dev::Renderer::Get().IsInit())
    {
        LLGL::Log::Errorf("Error: dev::Renderer is not initialized\n");
        return;
    }

    while(window->PollEvents())
    {
        dev::Multithreading::Get().Update();

        dev::Renderer::Get().RenderPass(
            [&](auto commandBuffer) { mesh->BindBuffers(commandBuffer); },
            {
                { 0, dev::Renderer::Get().GetMatricesBuffer() },
                { 1, texture->texture },
                { 2, sampler }
            },
            [&](auto commandBuffer) { mesh->Draw(commandBuffer); },
            pipeline
        );

        dev::Renderer::Get().Present();

        if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::Escape))
            break;

        degrees = (dev::Mouse::GetPosition().x - 400.0) / 100.0;

        matrices->Rotate(glm::radians(degrees), { 0.0f, 1.0f, 0.0f });

        window->SwapBuffers();
    }
}

void CubeApp::LoadShaders()
{
    vertexShader = dev::Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/vertex.vert");
    fragmentShader = dev::Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/fragment.frag");
}

void CubeApp::LoadTextures()
{
    texture = dev::TextureManager::Get().LoadTexture("../textures/tex.jpg");
    sampler = dev::TextureManager::Get().GetAnisotropySampler();
}

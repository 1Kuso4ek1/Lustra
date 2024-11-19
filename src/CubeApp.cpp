#include <CubeApp.hpp>

CubeApp::CubeApp()
{
    LLGL::Log::RegisterCallbackStd();

    window = std::make_shared<glfw::Window>(LLGL::Extent2D{ 800, 800 }, "LLGLTest");

    Renderer::Get().InitSwapChain({ 800, 800 }, window);

    LoadShaders();
    LoadTextures();

    (mesh = std::make_unique<Mesh>())->CreateCube();
    
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

    while(window->PollEvents())
    {
        Multithreading::Get().Update();

        Renderer::Get().RenderPass(
            [&](auto commandBuffer) { mesh->BindBuffers(commandBuffer); },
            {
                { 0, Renderer::Get().GetMatricesBuffer() },
                { 1, texture->texture },
                { 2, sampler }
            },
            [&](auto commandBuffer) { mesh->Draw(commandBuffer); },
            pipeline
        );

        Renderer::Get().Present();

        matrices->Rotate(glm::radians(degrees), { 0.0f, 1.0f, 0.0f });

        window->SwapBuffers();
    }
}

void CubeApp::LoadShaders()
{
    vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/vertex.vert");
    fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/fragment.frag");
}

void CubeApp::LoadTextures()
{
    texture = TextureManager::Get().LoadTexture("../textures/tex.jpg");
    sampler = TextureManager::Get().GetAnisotropySampler();
}

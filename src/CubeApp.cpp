#include <CubeApp.hpp>

CubeApp::CubeApp()
{
    LLGL::Log::RegisterCallbackStd();

    window = std::make_shared<dev::Window>(LLGL::Extent2D{ 1280, 720 }, "LLGLTest");

    dev::Renderer::Get().InitSwapChain(window);

    LoadShaders();
    LoadTextures();

    InitImGui();

    (mesh = std::make_unique<dev::Mesh>())->CreateCube();

    pipeline = dev::Renderer::Get().CreatePipelineState(vertexShader, fragmentShader);
    matrices = dev::Renderer::Get().GetMatrices();

    matrices->GetProjection() = glm::perspective(glm::radians(90.0f), 1280.0f / 720.0f, 0.1f, 100.0f);
    matrices->GetView() = glm::lookAt(glm::vec3(0.f, 0.f, 5.f), { 0, 0, 0.f }, glm::vec3(0.f, 1.f, 0.f));
}

CubeApp::~CubeApp()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void CubeApp::Run()
{
    if(!dev::Renderer::Get().IsInit())
    {
        LLGL::Log::Errorf("Error: Renderer is not initialized\n");
        return;
    }

    while(window->PollEvents())
    {
        LLGL::Surface::ProcessEvents();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();
        ImGui::Render();

        dev::Multithreading::Get().Update();

        dev::Renderer::Get().RenderPass(
            [&](auto commandBuffer)
            {
                mesh->BindBuffers(commandBuffer);
            },
            {
                { 0, dev::Renderer::Get().GetMatricesBuffer() },
                { 1, texture->texture },
                { 2, sampler }
            },
            [&](auto commandBuffer)
            {
                mesh->Draw(commandBuffer);
            },
            pipeline
        );

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        dev::Renderer::Get().Present();

        if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::Escape))
            break;

        if(dev::Mouse::IsButtonPressed(dev::Mouse::Button::Right))
            degrees = (dev::Mouse::GetPosition().x - 640.0) / 100.0;

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

void CubeApp::InitImGui()
{
    IMGUI_CHECKVERSION();
    
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(window->GetGLFWWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

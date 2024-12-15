#include <CubeApp.hpp>

CubeApp::CubeApp()
{
    LLGL::Log::RegisterCallbackStd(LLGL::Log::StdOutFlags::Colored);

    dev::ScopedTimer timer("Engine initialization");

    window = std::make_shared<dev::Window>(LLGL::Extent2D{ 1280, 720 }, "LLGLTest");

    if(!dev::Renderer::Get().IsInit())
        return;

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
    if(dev::Renderer::Get().IsInit())
    {
        DestroyImGui();

        dev::Renderer::Get().Unload();
    }
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
        
        dev::Multithreading::Get().Update();

        Draw();

        if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::Escape))
            break;

        if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::F11) && keyboardTimer.GetElapsedSeconds() > 0.2f)
        {
            window->SetFullscreen(!window->IsFullscreen());
            keyboardTimer.Reset();
        }

        if(dev::Mouse::IsButtonPressed(dev::Mouse::Button::Right))
            degrees = (dev::Mouse::GetPosition().x - 640.0) / 100.0;

        matrices->GetModel() = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
    }
}

void CubeApp::LoadShaders()
{
    vertexShader = dev::Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/vertex.vert");
    fragmentShader = dev::Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/fragment.frag");
}

void CubeApp::LoadTextures()
{
    texture = dev::TextureManager::Get().LoadTexture("../resources/textures/tex.jpg");
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

    ImFontConfig config;
    config.OversampleH = 8;
    config.OversampleV = 8;

    SetupImGuiStyle();

    io.Fonts->AddFontFromFileTTF("../resources/fonts/OpenSans-Regular.ttf", 18.0f, &config);

    ImGui_ImplGlfw_InitForOpenGL(window->GetGLFWWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

void CubeApp::DestroyImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void CubeApp::NewImGuiFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void CubeApp::SetupImGuiStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    
    style.WindowPadding = ImVec2(15, 15);
    style.WindowRounding = 5.0f;
    style.FramePadding = ImVec2(5, 5);
    style.FrameRounding = 4.0f;
    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 15.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabMinSize = 5.0f;
    style.GrabRounding = 3.0f;
    
    style.Colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

    style.Colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
    style.Colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    
    style.Colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
    style.Colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

    style.Colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

    style.Colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    style.Colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
    style.Colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

    style.Colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
}

void CubeApp::DrawImGui()
{
    NewImGuiFrame();

    ImGui::ShowDemoWindow();

    ImGui::Begin("Cube Rotation");

    ImGui::Text("Choose Axis for Rotation:");
    ImGui::SliderFloat3("Axis", &axis[0], -1.0f, 1.0f);
    
    ImGui::Text("Choose Angle for Rotation:");
    ImGui::SliderFloat("Angle", &angle, 0.0f, 360.0f);

    ImGui::End();

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void CubeApp::Draw()
{
    dev::Renderer::Get().Begin();

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

    dev::Renderer::Get().End();

    dev::Renderer::Get().Submit();

    DrawImGui();

    dev::Renderer::Get().Present();
}

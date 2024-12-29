#include <SceneTestApp.hpp>

SceneTestApp::SceneTestApp()
{
    LLGL::Log::RegisterCallbackStd(LLGL::Log::StdOutFlags::Colored);

    dev::ScopedTimer timer("Engine initialization");

    window = std::make_shared<dev::Window>(LLGL::Extent2D{ 1280, 720 }, "LLGLTest", 1, true);

    if(!dev::Renderer::Get().IsInit())
        return;

    dev::Renderer::Get().InitSwapChain(window);

    dev::ImGuiManager::Get().Init(window->GetGLFWWindow(), "../resources/fonts/OpenSans-Regular.ttf");

    LoadShaders();
    LoadTextures();

    (mesh = std::make_shared<dev::Mesh>())->CreateCube();

    pipeline = dev::Renderer::Get().CreatePipelineState(vertexShader, fragmentShader);
    matrices = dev::Renderer::Get().GetMatrices();

    scene.SetRenderer(std::make_shared<dev::DeferredRenderer>());

    CreateEntities();
}

SceneTestApp::~SceneTestApp()
{
    if(dev::Renderer::Get().IsInit())
    {
        dev::ImGuiManager::Get().Destroy();

        dev::Renderer::Get().Unload();
    }
}

void SceneTestApp::Run()
{
    if(!dev::Renderer::Get().IsInit())
    {
        LLGL::Log::Errorf(LLGL::Log::ColorFlags::StdError, "Error: Renderer is not initialized\n");
        return;
    }

    scene.Start();

    while(window->PollEvents())
    {
        LLGL::Surface::ProcessEvents();
        
        dev::Multithreading::Get().Update();

        scene.Update(deltaTimeTimer.GetElapsedSeconds());
        deltaTimeTimer.Reset();

        Draw();

        if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::Escape))
            break;

        if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::F11) && keyboardTimer.GetElapsedSeconds() > 0.2f)
        {
            window->SetFullscreen(!window->IsFullscreen());
            keyboardTimer.Reset();
        }
    }
}

void SceneTestApp::LoadShaders()
{
    vertexShader = dev::Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/vertex.vert");
    fragmentShader = dev::Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/fragment.frag");
}

void SceneTestApp::LoadTextures()
{
    texture = dev::TextureManager::Get().LoadTexture("../resources/textures/tex.jpg");
}

void SceneTestApp::CreateEntities()
{
    CreateCubeEntity();
    CreateCameraEntity();
    CreatePostProcessingEntity();
    CreateLightEntity();
    CreateLight1Entity();
}

void SceneTestApp::CreateCubeEntity()
{
    entity = scene.CreateEntity();

    entity.AddComponent<dev::NameComponent>().name = "Cube";
    entity.AddComponent<dev::TransformComponent>();
    entity.AddComponent<dev::MeshComponent>().meshes.push_back(mesh);
    entity.AddComponent<dev::MeshRendererComponent>().materials.push_back(texture);
    entity.AddComponent<dev::PipelineComponent>().pipeline = pipeline;
    
    auto& script = entity.AddComponent<dev::ScriptComponent>();

    script.start = []() {};
    script.update = [](dev::Entity entity, float deltaTime)
    {
        entity.GetComponent<dev::TransformComponent>().rotation.y += 10.0f * deltaTime;
    };
}

void SceneTestApp::CreateCameraEntity()
{
    camera = scene.CreateEntity();

    camera.AddComponent<dev::NameComponent>().name = "Camera";
    camera.AddComponent<dev::TransformComponent>().position = { 0.0f, 0.0f, 5.0f };
    
    auto& cameraComponent = camera.AddComponent<dev::CameraComponent>();
    
    cameraComponent.camera.SetViewport(window->GetContentSize());
    cameraComponent.camera.SetPerspective();

    auto& cameraScript = camera.AddComponent<dev::ScriptComponent>();

    cameraScript.start = []() {};
    cameraScript.update = [&](dev::Entity entity, float deltaTime)
    {
        if(dev::Mouse::IsButtonPressed(dev::Mouse::Button::Right))
        {
            dev::Mouse::SetCursorVisible(false);

            auto& transform = entity.GetComponent<dev::TransformComponent>();
            auto rotation = glm::quat(glm::radians(transform.rotation));

            if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::W))
                transform.position -= rotation * glm::vec3(0.0f, 0.0f, 0.001f);
            if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::S))
                transform.position += rotation * glm::vec3(0.0f, 0.0f, 0.001f);
            if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::A))
                transform.position -= rotation * glm::vec3(0.001f, 0.0f, 0.0f);
            if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::D))
                transform.position += rotation * glm::vec3(0.001f, 0.0f, 0.0f);

            glm::vec2 center(window->GetContentSize().width / 2.0f, window->GetContentSize().height / 2.0f);
            glm::vec2 delta = center - dev::Mouse::GetPosition();

            transform.rotation.x += delta.y / 100.0f;
            transform.rotation.y += delta.x / 100.0f;

            dev::Mouse::SetPosition(center);
        }
        else
            dev::Mouse::SetCursorVisible();
    };
}

void SceneTestApp::CreatePostProcessingEntity()
{
    postProcessing = scene.CreateEntity();

    postProcessing.AddComponent<dev::NameComponent>().name = "PostProcessing";
    postProcessing.AddComponent<dev::ACESTonemappingComponent>();
}

void SceneTestApp::CreateLightEntity()
{
    light = scene.CreateEntity();

    light.AddComponent<dev::NameComponent>().name = "Light";
    light.AddComponent<dev::TransformComponent>().position = { 5.0f, 5.0f, 5.0f };
    light.AddComponent<dev::LightComponent>().intensity = 1.0f;
}

void SceneTestApp::CreateLight1Entity()
{
    light1 = scene.CreateEntity();

    light1.AddComponent<dev::NameComponent>().name = "Light1";
    light1.AddComponent<dev::TransformComponent>().position = { -5.0f, 5.0f, 5.0f };
    light1.AddComponent<dev::LightComponent>().color = { 1.0f, 0.0f, 0.0f };
    light1.GetComponent<dev::LightComponent>().intensity = 1.0f;
}

void SceneTestApp::DrawImGui()
{
    dev::ImGuiManager::Get().NewFrame();

    ImGui::ShowDemoWindow();

    ImGui::ShowMetricsWindow();

    ImGui::Begin("Scene");

    if(ImGui::CollapsingHeader("Cube"))
    {
        ImGui::Indent();
        dev::DrawEntityUI<dev::NameComponent, dev::TransformComponent>(scene.GetRegistry(), entity);
        ImGui::Unindent();
    }

    if(ImGui::CollapsingHeader("Camera"))
    {
        ImGui::Indent();
        dev::DrawEntityUI<dev::NameComponent, dev::TransformComponent, dev::CameraComponent>(scene.GetRegistry(), camera);
        ImGui::Unindent();
    }

    if(ImGui::CollapsingHeader("PostProcessing"))
    {
        ImGui::Indent();
        dev::DrawEntityUI<dev::NameComponent, dev::ACESTonemappingComponent>(scene.GetRegistry(), postProcessing);
        ImGui::Unindent();
    }

    if(ImGui::CollapsingHeader("Light"))
    {
        ImGui::Indent();
        dev::DrawEntityUI<dev::NameComponent, dev::TransformComponent, dev::LightComponent>(scene.GetRegistry(), light);
        ImGui::Unindent();
    }

    if(ImGui::CollapsingHeader("Light1"))
    {
        ImGui::Indent();
        dev::DrawEntityUI<dev::NameComponent, dev::TransformComponent, dev::LightComponent>(scene.GetRegistry(), light1);
        ImGui::Unindent();
    }
    
    ImGui::End();

    ImGui::Begin("Texture viewer");

    static uint32_t startIndex = 12;
    static uint32_t texturesCount = 5;

    ImGui::Text("Start index");
    if(ImGui::Button("-") && startIndex > 1) startIndex--;
        ImGui::SameLine();

        ImGui::SetNextItemWidth(30); 
        ImGui::InputScalar("##Input", ImGuiDataType_U32, &startIndex, NULL, 0);
        
        ImGui::SameLine();
    if(ImGui::Button("+")) startIndex++;

    ImGui::Text("Textures count");
    if(ImGui::Button("-##") && texturesCount > 1) texturesCount--;
        ImGui::SameLine();

        ImGui::SetNextItemWidth(30); 
        ImGui::InputScalar("##Input1", ImGuiDataType_U32, &texturesCount, NULL, 0);
        
        ImGui::SameLine();
    if(ImGui::Button("+##")) texturesCount++;
    
    for(uint32_t i = startIndex; i < startIndex + texturesCount; i++)
        ImGui::Image(i, ImVec2(320, 180));
    
    ImGui::End();

    dev::ImGuiManager::Get().Render();
}

void SceneTestApp::Draw()
{
    scene.Draw();

    DrawImGui();

    dev::Renderer::Get().Present();
}

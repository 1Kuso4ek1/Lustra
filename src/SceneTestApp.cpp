#include <SceneTestApp.hpp>

SceneTestApp::SceneTestApp()
{
    LLGL::Log::RegisterCallbackStd(LLGL::Log::StdOutFlags::Colored);

    dev::ScopedTimer timer("Engine initialization");

    window = std::make_shared<dev::Window>(LLGL::Extent2D{ 1280, 720 }, "LLGLTest", 1, false);

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
    CreateSkyEntity();
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
        static auto lerp = [](float a, float b, float t)
        {
            return a + t * (b - a);
        };

        static float speed = 0.0f;

        static glm::vec3 movement = glm::vec3(0.0f);

        if(dev::Mouse::IsButtonPressed(dev::Mouse::Button::Right))
        {
            dev::Mouse::SetCursorVisible(false);

            auto& transform = entity.GetComponent<dev::TransformComponent>();
            auto rotation = glm::quat(glm::radians(transform.rotation));

            glm::vec3 input = glm::vec3(0.0f);

            if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::W))
                input -= rotation * glm::vec3(0.0f, 0.0f, 1.0f);
            if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::S))
                input += rotation * glm::vec3(0.0f, 0.0f, 1.0f);
            if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::A))
                input -= rotation * glm::vec3(1.0f, 0.0f, 0.0f);
            if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::D))
                input += rotation * glm::vec3(1.0f, 0.0f, 0.0f);

            if(glm::length(input) > 0.1f)
            {
                speed = lerp(speed, 3.0f, 0.01f);
                movement = input;
            }
            else
                speed = lerp(speed, 0.0f, 0.01f);

            if(speed > 0.0f)
                transform.position += glm::normalize(movement) * deltaTime * speed;

            glm::vec2 center(window->GetContentSize().width / 2.0f, window->GetContentSize().height / 2.0f);
            glm::vec2 delta = center - dev::Mouse::GetPosition();

            transform.rotation.x += delta.y / 100.0f;
            transform.rotation.y += delta.x / 100.0f;

            transform.rotation.x = glm::clamp(transform.rotation.x, -89.0f, 89.0f);

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

void SceneTestApp::CreateSkyEntity()
{
    sky = scene.CreateEntity();

    sky.AddComponent<dev::NameComponent>().name = "Sky";
    sky.AddComponent<dev::MeshComponent>().meshes.push_back(std::make_shared<dev::Mesh>());
    sky.GetComponent<dev::MeshComponent>().meshes[0]->CreateCube();

    sky.AddComponent<dev::ProceduralSkyComponent>();
}

void SceneTestApp::DrawImGui()
{
    dev::ImGuiManager::Get().NewFrame();

    ImGui::ShowDemoWindow();

    ImGui::ShowMetricsWindow();

    ImGui::Begin("Scene");

    static std::vector<dev::Entity> list { entity, camera, postProcessing, light, light1, sky };

    for(auto entity : list)
    {
        if(ImGui::TreeNodeEx(entity.GetComponent<dev::NameComponent>().name.c_str()))
        {
            ImGui::Indent();
            dev::DrawEntityUI<dev::NameComponent,
                              dev::TransformComponent,
                              dev::CameraComponent,
                              dev::LightComponent,
                              dev::ACESTonemappingComponent,
                              dev::ProceduralSkyComponent>(scene.GetRegistry(), entity);
            ImGui::Unindent();

            ImGui::TreePop();
        }
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

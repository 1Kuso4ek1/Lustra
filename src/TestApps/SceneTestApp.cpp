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

    dev::EventManager::Get().AddListener(dev::Event::Type::WindowResize, this);

    SetupAssetManager();

    LoadShaders();
    LoadTextures();

    deferredRenderer = std::make_shared<dev::DeferredRenderer>();

    scene.SetRenderer(deferredRenderer);

    CreateEntities();

    CreateRenderTarget();

    list = { rifle, camera, postProcessing, light, light1, sky };
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

void SceneTestApp::SetupAssetManager()
{
    dev::AssetManager::Get().SetAssetsDirectory("../resources/");
    dev::AssetManager::Get().AddLoader<dev::TextureAsset, dev::TextureLoader>("textures");
    dev::AssetManager::Get().AddLoader<dev::ModelAsset, dev::ModelLoader>("models");
}

void SceneTestApp::LoadShaders()
{
    vertexShader = dev::Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/vertex.vert");
    fragmentShader = dev::Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/fragment.frag");

    pipeline = dev::Renderer::Get().CreatePipelineState(vertexShader, fragmentShader);
}

void SceneTestApp::LoadTextures()
{
    texture = dev::AssetManager::Get().Load<dev::TextureAsset>("tex.jpg", true);

    metal = dev::AssetManager::Get().Load<dev::TextureAsset>("Metall_ak-47_Base_Color.png", true);
    wood = dev::AssetManager::Get().Load<dev::TextureAsset>("Wood_ak-47_Base_Color.png", true);
}

void SceneTestApp::CreateEntities()
{
    CreateRifleEntity();
    CreateCameraEntity();
    CreatePostProcessingEntity();
    CreateLightEntity();
    CreateLight1Entity();
    CreateSkyEntity();
}

void SceneTestApp::CreateRifleEntity()
{
    rifle = scene.CreateEntity();

    rifle.AddComponent<dev::NameComponent>().name = "Rifle";
    rifle.AddComponent<dev::TransformComponent>().rotation = { -90.0f, 180.0f, 0.0f };
    rifle.AddComponent<dev::MeshComponent>().meshes = dev::AssetManager::Get().Load<dev::ModelAsset>("ak-47.fbx", true)->meshes;
    rifle.AddComponent<dev::MeshRendererComponent>().materials = { wood, metal };
    rifle.AddComponent<dev::PipelineComponent>().pipeline = pipeline;
    
    auto& script = rifle.AddComponent<dev::ScriptComponent>();

    script.start = []() {};
    script.update = [](dev::Entity entity, float deltaTime)
    {
        if(entity.HasComponent<dev::TransformComponent>() && dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::Q))
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

        static auto& transform = entity.GetComponent<dev::TransformComponent>();

        static float speed = 0.0f;

        static glm::vec3 movement = glm::vec3(0.0f);

        if(dev::Mouse::IsButtonPressed(dev::Mouse::Button::Right))
        {
            dev::Mouse::SetCursorVisible(false);

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
    postProcessing.AddComponent<dev::ACESTonemappingComponent>(LLGL::Extent2D{ 1280, 720 }, false);
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
    sky.AddComponent<dev::MeshComponent>().meshes = dev::AssetManager::Get().Load<dev::ModelAsset>("cube")->meshes;

    sky.AddComponent<dev::ProceduralSkyComponent>();
}

void SceneTestApp::CreateRenderTarget(const LLGL::Extent2D& resolution)
{
    LLGL::TextureDescriptor colorAttachmentDesc =
    {
        .type = LLGL::TextureType::Texture2D,
        .bindFlags = LLGL::BindFlags::ColorAttachment,
        .format = LLGL::Format::RGBA8UNorm,
        .extent = { resolution.width, resolution.height, 1 },
        .mipLevels = 1,
        .samples = 1
    };

    viewportAttachment = dev::Renderer::Get().CreateTexture(colorAttachmentDesc);
    viewportRenderTarget = dev::Renderer::Get().CreateRenderTarget(resolution, { viewportAttachment });

    LLGL::OpenGL::ResourceNativeHandle nativeHandle;
    viewportAttachment->GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
    nativeViewportAttachment = nativeHandle.id;
}

void SceneTestApp::DrawImGui()
{
    dev::ImGuiManager::Get().NewFrame();

    ImGuizmo::BeginFrame();

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

    ImGui::ShowDemoWindow();

    ImGui::ShowMetricsWindow();

    DrawSceneTree();
    DrawPropertiesWindow();
    DrawImGuizmoControls();
    DrawTextureViewer();
    DrawViewport();

    dev::ImGuiManager::Get().Render();
}

void SceneTestApp::DrawSceneTree()
{
    ImGui::Begin("Scene");

    for(auto entity : list)
    {
        std::string name = (entity.HasComponent<dev::NameComponent>() ? entity.GetComponent<dev::NameComponent>().name : "Entity");

        ImGui::PushID((uint64_t)entity);

        if(ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_OpenOnDoubleClick))
        {
            if(ImGui::IsItemClicked())
                selectedEntity = entity;

            ImGui::TreePop();
        }
        
        ImGui::PopID();

        if(ImGui::IsItemClicked())
            selectedEntity = entity;
    }
    
    ImGui::End();
}

void SceneTestApp::DrawPropertiesWindow()
{
    ImGui::Begin("Properties");

    if(selectedEntity)
    {
        dev::DrawEntityUI<dev::NameComponent,
                          dev::TransformComponent,
                          dev::CameraComponent,
                          dev::LightComponent,
                          dev::ACESTonemappingComponent,
                          dev::ProceduralSkyComponent>(scene.GetRegistry(), selectedEntity);

        ImGui::Separator();
            
        if(ImGui::Button("Remove entity"))
        {
            auto it = std::find(list.begin(), list.end(), (entt::entity)selectedEntity);
            if (it != list.end())
                list.erase(it);

            scene.RemoveEntity(selectedEntity);
        }
    }
    else
        ImGui::TextDisabled("Select an entity...");

    ImGui::End();
}

void SceneTestApp::DrawImGuizmoControls()
{
    ImGui::Begin("ImGuizmo Controls");

    if(ImGui::RadioButton("Translate", currentOperation == ImGuizmo::OPERATION::TRANSLATE))
        currentOperation = ImGuizmo::OPERATION::TRANSLATE;
    
    ImGui::SameLine();
    if(ImGui::RadioButton("Rotate", currentOperation == ImGuizmo::OPERATION::ROTATE))
        currentOperation = ImGuizmo::OPERATION::ROTATE;
    
    ImGui::SameLine();
    if(ImGui::RadioButton("Scale", currentOperation == ImGuizmo::OPERATION::SCALE))
        currentOperation = ImGuizmo::OPERATION::SCALE;

    ImGui::DragFloat3("Snap", snap, 0.1f, 0.0f, 45.0f);

    ImGui::End();
}

void SceneTestApp::DrawImGuizmo()
{
    if(selectedEntity) 
    {
        if(selectedEntity.HasComponent<dev::TransformComponent>())
        {
            auto viewMatrix = dev::Renderer::Get().GetMatrices()->GetView();
            auto projectionMatrix = dev::Renderer::Get().GetMatrices()->GetProjection();

            auto modelMatrix = selectedEntity.GetComponent<dev::TransformComponent>().GetTransform();

            ImGuizmo::SetDrawlist();
            
            ImGuizmo::SetRect(
                ImGui::GetWindowPos().x,
                ImGui::GetWindowPos().y,
                ImGui::GetWindowWidth(),
                ImGui::GetWindowHeight()
            );
            
            //ImGuizmo::SetRect(0, 0, window->GetContentSize().width, window->GetContentSize().height);

            ImGuizmo::Manipulate(
                glm::value_ptr(viewMatrix),
                glm::value_ptr(projectionMatrix),
                currentOperation,
                ImGuizmo::MODE::WORLD,
                glm::value_ptr(modelMatrix),
                nullptr,
                dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::LeftControl) ? snap : nullptr
            );

            /* ImGuizmo::DrawGrid(
                glm::value_ptr(viewMatrix),
                glm::value_ptr(projectionMatrix),
                glm::value_ptr(glm::mat4(1.0)),
                10.0f
            ); */

            selectedEntity.GetComponent<dev::TransformComponent>().SetTransform(modelMatrix);

            ImGuizmo::Enable(true);
        }
    }
    else
        ImGuizmo::Enable(false);
}

void SceneTestApp::DrawTextureViewer()
{
    ImGui::Begin("Textures");

    auto& assets = dev::AssetManager::Get().GetAssets();

    for(auto const& [path, asset] : assets)
    {
        auto textureAsset = std::dynamic_pointer_cast<dev::TextureAsset>(asset);

        if(textureAsset)
        {
            ImGui::PushID(path.string().c_str());

            ImGui::Image(textureAsset->nativeHandle, ImVec2(128, 128));

            ImGui::PopID();
        }
    }

    ImGui::End();
}

void SceneTestApp::DrawViewport()
{
    static dev::Timer eventTimer;
    static ImGuiWindowFlags flags = 0;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::Begin("Viewport", nullptr, flags);

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    flags = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(window->InnerRect.Min, window->InnerRect.Max) ? ImGuiWindowFlags_NoMove : 0;

    auto size = window->InnerRect.GetSize();
    if((size.x != viewportRenderTarget->GetResolution().width || 
        size.y != viewportRenderTarget->GetResolution().height) &&
        eventTimer.GetElapsedSeconds() > 0.2f)
    {
        dev::EventManager::Get().Dispatch(
            std::make_unique<dev::WindowResizeEvent>(
                LLGL::Extent2D{ (uint32_t)size.x, (uint32_t)size.y }
            )
        );

        eventTimer.Reset();
    }

    ImGui::Image(nativeViewportAttachment, window->InnerRect.GetSize());

    DrawImGuizmo();

    ImGui::End();

    ImGui::PopStyleVar();
}

void SceneTestApp::Draw()
{
    scene.Draw(viewportRenderTarget);

    ClearScreen();

    DrawImGui();

    dev::Renderer::Get().Present();
}

void SceneTestApp::ClearScreen()
{
    dev::Renderer::Get().Begin();

    dev::Renderer::Get().RenderPass(
        [](auto){}, {}, 
        [](auto commandBuffer)
        {
            commandBuffer->Clear(LLGL::ClearFlags::ColorDepth);
        },
        nullptr
    );

    dev::Renderer::Get().End();

    dev::Renderer::Get().Submit();
}

void SceneTestApp::OnEvent(dev::Event& event)
{
    if(event.GetType() == dev::Event::Type::WindowResize)
    {
        auto resizeEvent = dynamic_cast<dev::WindowResizeEvent*>(&event);

        CreateRenderTarget(resizeEvent->GetSize());
    }
}

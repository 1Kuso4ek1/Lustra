#include <SceneTestApp.hpp>
#include <fstream>

SceneTestApp::SceneTestApp()
{
    LLGL::Log::RegisterCallbackStd(LLGL::Log::StdOutFlags::Colored);

    dev::ScopedTimer timer("Engine initialization");

    window = std::make_shared<dev::Window>(LLGL::Extent2D{ 1280, 720 }, "LLGLTest", 1, true);

    if(!dev::Renderer::Get().IsInit())
        return;

    dev::Renderer::Get().InitSwapChain(window);

    dev::ImGuiManager::Get().Init(
        window->GetGLFWWindow(),
        "../resources/fonts/OpenSans-Regular.ttf",
        "../resources/layout/editor_layout.ini"
    );

    dev::PhysicsManager::Get().Init();

    dev::EventManager::Get().AddListener(dev::Event::Type::WindowResize, this);

    SetupAssetManager();

    LoadTextures();

    deferredRenderer = std::make_shared<dev::DeferredRenderer>();

    scene = std::make_shared<dev::Scene>(deferredRenderer);

    CreateRenderTarget();

    CreateEntities();

    sceneAsset = std::make_shared<dev::SceneAsset>(scene);

    //dev::AssetManager::Get().Write(sceneAsset, "scene.json", true);

    /* auto sceneAsset = dev::AssetManager::Get().Load<dev::SceneAsset>("scene.json", true);
    scene = sceneAsset->scene;
    scene->SetRenderer(deferredRenderer);

    for(auto entity: scene->GetRegistry().view<entt::entity>()) 
        list.emplace_back(entity, scene.get()); */
    //list = { rifle, camera, editorCamera, postProcessing, light, light1, sky };
    UpdateList();
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

    scene->Start();

    while(window->PollEvents())
    {
        LLGL::Surface::ProcessEvents();
        
        dev::Multithreading::Get().Update();

        if(playing && !paused)
            scene->Update(deltaTimeTimer.GetElapsedSeconds());
        else
        {
            editorCamera.GetComponent<dev::CameraComponent>().active = true;
            editorCamera.GetComponent<dev::ScriptComponent>().update(editorCamera, deltaTimeTimer.GetElapsedSeconds());
        }

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
    dev::AssetManager::Get().AddLoader<dev::MaterialAsset, dev::MaterialLoader>("materials");
    dev::AssetManager::Get().AddLoader<dev::ModelAsset, dev::ModelLoader>("models");
    dev::AssetManager::Get().AddLoader<dev::ScriptAsset, dev::ScriptLoader>("scripts");
    dev::AssetManager::Get().AddLoader<dev::VertexShaderAsset, dev::VertexShaderLoader>("../shaders");
    dev::AssetManager::Get().AddLoader<dev::FragmentShaderAsset, dev::FragmentShaderLoader>("../shaders");
    dev::AssetManager::Get().AddLoader<dev::SceneAsset, dev::SceneLoader>("scenes");
}

void SceneTestApp::LoadTextures()
{
    texture = dev::AssetManager::Get().Load<dev::TextureAsset>("tex.jpg", true);

    metal = dev::AssetManager::Get().Load<dev::TextureAsset>("Metall_ak-47_Base_Color.png", true);
    wood = dev::AssetManager::Get().Load<dev::TextureAsset>("Wood_ak-47_Base_Color.png", true);

    /* ak47Metal = std::make_shared<dev::MaterialAsset>();
    ak47Wood = std::make_shared<dev::MaterialAsset>();

    ak47Metal->albedo = metal;
    ak47Wood->albedo = wood;

    dev::AssetManager::Get().Write(ak47Metal, "ak47Metal", true);
    dev::AssetManager::Get().Write(ak47Wood, "ak47Wood", true); */

    ak47Metal = dev::AssetManager::Get().Load<dev::MaterialAsset>("ak47Metal", true);
    ak47Wood = dev::AssetManager::Get().Load<dev::MaterialAsset>("ak47Wood", true);

    fileIcon = dev::AssetManager::Get().Load<dev::TextureAsset>("icons/file.png", true);
    folderIcon = dev::AssetManager::Get().Load<dev::TextureAsset>("icons/folder.png", true);
    textureIcon = dev::AssetManager::Get().Load<dev::TextureAsset>("icons/texture.png", true);
    materialIcon = dev::AssetManager::Get().Load<dev::TextureAsset>("icons/material.png", true);
    modelIcon = dev::AssetManager::Get().Load<dev::TextureAsset>("icons/model.png", true);
    scriptIcon = dev::AssetManager::Get().Load<dev::TextureAsset>("icons/script.png", true);
    sceneIcon = dev::AssetManager::Get().Load<dev::TextureAsset>("icons/scene.png", true);

    playIcon = dev::AssetManager::Get().Load<dev::TextureAsset>("icons/play.png", true);
    pauseIcon = dev::AssetManager::Get().Load<dev::TextureAsset>("icons/pause.png", true);
    stopIcon = dev::AssetManager::Get().Load<dev::TextureAsset>("icons/stop.png", true);
    buildIcon = dev::AssetManager::Get().Load<dev::TextureAsset>("icons/build.png", true);

    lightIcon = dev::AssetManager::Get().Load<dev::TextureAsset>("icons/light.png", true);

    assetIcons = 
    {
        { dev::Asset::Type::Texture, textureIcon },
        { dev::Asset::Type::Material, materialIcon },
        { dev::Asset::Type::Model, modelIcon },
        { dev::Asset::Type::Script, scriptIcon },
        { dev::Asset::Type::Scene, sceneIcon },
        { dev::Asset::Type::Unknown, fileIcon }
    };
}

void SceneTestApp::CreateEntities()
{
    CreateRifleEntity();
    CreateCameraEntity();
    CreateEditorCameraEntity();
    CreatePostProcessingEntity();
    CreateLightEntity();
    CreateLight1Entity();
    CreateSkyEntity();
}

void SceneTestApp::CreateRifleEntity()
{
    auto rifle = scene->CreateEntity();

    rifle.AddComponent<dev::NameComponent>().name = "Rifle";
    rifle.AddComponent<dev::TransformComponent>().rotation = { -90.0f, 180.0f, 0.0f };
    rifle.AddComponent<dev::MeshComponent>().model = dev::AssetManager::Get().Load<dev::ModelAsset>("ak-47.fbx", true);
    rifle.AddComponent<dev::MeshRendererComponent>().materials = { ak47Wood, ak47Metal };
    rifle.AddComponent<dev::PipelineComponent>(
        dev::AssetManager::Get().Load<dev::VertexShaderAsset>("vertex.vert", true),
        dev::AssetManager::Get().Load<dev::FragmentShaderAsset>("deferred.frag", true)
    );
    rifle.AddComponent<dev::ScriptComponent>();
}

void SceneTestApp::CreateCameraEntity()
{
    auto camera = scene->CreateEntity();

    camera.AddComponent<dev::NameComponent>().name = "Camera";
    camera.AddComponent<dev::TransformComponent>().position = { 5.0f, 5.0f, 5.0f };
    
    auto& cameraComponent = camera.AddComponent<dev::CameraComponent>();
    
    cameraComponent.camera.SetViewport(window->GetContentSize());
    cameraComponent.camera.SetPerspective();
}

void SceneTestApp::CreateEditorCameraEntity()
{
    editorCamera = scene->CreateEntity();

    editorCamera.AddComponent<dev::NameComponent>().name = "EditorCamera";
    editorCamera.AddComponent<dev::TransformComponent>().position = { 0.0f, 0.0f, 5.0f };
    
    auto& cameraComponent = editorCamera.AddComponent<dev::CameraComponent>();
    
    cameraComponent.camera.SetViewport(window->GetContentSize());
    cameraComponent.camera.SetPerspective();
    cameraComponent.active = true;

    UpdateEditorCameraScript();
}

void SceneTestApp::CreatePostProcessingEntity()
{
    auto postProcessing = scene->CreateEntity();

    postProcessing.AddComponent<dev::NameComponent>().name = "PostProcessing";
    postProcessing.AddComponent<dev::TonemapComponent>(LLGL::Extent2D{ 1280, 720 });
    postProcessing.AddComponent<dev::BloomComponent>(LLGL::Extent2D{ 1280, 720 });
    postProcessing.AddComponent<dev::GTAOComponent>(LLGL::Extent2D{ 1280, 720 });
    postProcessing.AddComponent<dev::SSRComponent>(LLGL::Extent2D{ 1280, 720 });
}

void SceneTestApp::CreateLightEntity()
{
    auto light = scene->CreateEntity();

    light.AddComponent<dev::NameComponent>().name = "Light";
    light.AddComponent<dev::TransformComponent>().position = { 5.0f, 5.0f, 5.0f };
    light.AddComponent<dev::LightComponent>().intensity = 1.0f;
}

void SceneTestApp::CreateLight1Entity()
{
    auto light1 = scene->CreateEntity();

    light1.AddComponent<dev::NameComponent>().name = "Light1";
    light1.AddComponent<dev::TransformComponent>().position = { -5.0f, 5.0f, 5.0f };
    light1.AddComponent<dev::LightComponent>().color = { 1.0f, 0.0f, 0.0f };
    light1.GetComponent<dev::LightComponent>().intensity = 1.0f;
}

void SceneTestApp::CreateSkyEntity()
{
    auto sky = scene->CreateEntity();
    
    sky.AddComponent<dev::NameComponent>().name = "Sky";
    sky.AddComponent<dev::MeshComponent>().model = dev::AssetManager::Get().Load<dev::ModelAsset>("cube", true);

    //sky.AddComponent<dev::ProceduralSkyComponent>(LLGL::Extent2D{ 1024, 1024 });
    sky.AddComponent<dev::HDRISkyComponent>(
        dev::AssetManager::Get().Load<dev::TextureAsset>("hdri/meadow_2_1k.hdr", true),
        LLGL::Extent2D{ 1024, 1024 }
    );
}

void SceneTestApp::UpdateList()
{
    list.clear();
            
    for(auto entity: scene->GetRegistry().view<entt::entity>()) 
        list.emplace_back(entity, scene.get());

    editorCamera = scene->GetEntity("EditorCamera");
}

void SceneTestApp::UpdateEditorCameraScript()
{
    auto& cameraScript = editorCamera.GetOrAddComponent<dev::ScriptComponent>();

    // Just a reminder that this is possible ;)
    /* auto& script = camera.AddComponent<dev::ScriptComponent>();
    script.script = dev::AssetManager::Get().Load<dev::ScriptAsset>("camera.as", true);

    dev::ScriptManager::Get().AddScript(script.script);
    dev::ScriptManager::Get().Build(); */

    cameraScript.update = [&](dev::Entity entity, float deltaTime)
    {
        static auto& transform = entity.GetComponent<dev::TransformComponent>();

        static float speed = 0.0f;

        static glm::vec3 movement = glm::vec3(0.0f);

        if(dev::Mouse::IsButtonPressed(dev::Mouse::Button::Right) && canMoveCamera)
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

            float lerpSpeed = glm::clamp(deltaTime * 5.0f, 0.0f, 1.0f);

            if(glm::length(input) > 0.1f)
            {
                speed = glm::mix(speed, 3.0f, lerpSpeed);
                movement = input;
            }
            else
                speed = glm::mix(speed, 0.0f, lerpSpeed);

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

void SceneTestApp::CreateModelEntity(dev::ModelAssetPtr model, bool relativeToCamera)
{
    auto entity = scene->CreateEntity();

    entity.AddComponent<dev::NameComponent>().name = "Model";
    entity.AddComponent<dev::TransformComponent>();
    entity.AddComponent<dev::MeshComponent>().model = model;
    entity.AddComponent<dev::MeshRendererComponent>();
    entity.AddComponent<dev::PipelineComponent>(
        dev::AssetManager::Get().Load<dev::VertexShaderAsset>("vertex.vert", true),
        dev::AssetManager::Get().Load<dev::FragmentShaderAsset>("deferred.frag", true)
    );
    
    auto& rigidBody = entity.AddComponent<dev::RigidBodyComponent>();

    auto& modelPos = entity.GetComponent<dev::TransformComponent>().position;

    if(relativeToCamera)
    {
        auto& cameraPos = editorCamera.GetComponent<dev::TransformComponent>().position;

        auto cameraOrient = glm::quat(glm::radians(editorCamera.GetComponent<dev::TransformComponent>().rotation));

        modelPos = cameraPos + cameraOrient * glm::vec3(0.0f, 0.0f, -5.0f);
    }

    auto settings =
        JPH::BodyCreationSettings(
            new JPH::EmptyShapeSettings(),
            { modelPos.x, modelPos.y, modelPos.z },
            { 0.0f, 0.0f, 0.0f, 1.0f },
            JPH::EMotionType::Dynamic,
            dev::Layers::moving
        );

    settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
    settings.mMassPropertiesOverride.mMass = 1.0f;

    rigidBody.body = dev::PhysicsManager::Get().CreateBody(settings);

    selectedEntity = entity;

    list.push_back(entity);
}

void SceneTestApp::CreateRenderTarget(const LLGL::Extent2D& resolution)
{
    if(viewportRenderTarget)
    {
        dev::Renderer::Get().Release(viewportAttachment);
        dev::Renderer::Get().Release(viewportRenderTarget);
    }

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

    ImGui::ShowMetricsWindow();

    DrawSceneTree();
    DrawPropertiesWindow();
    DrawImGuizmoControls();
    DrawExecutionControl();
    DrawAssetBrowser();

    if(selectedAsset)
    {
        if(selectedAsset->type == dev::Asset::Type::Material)
            DrawMaterialEditor(std::static_pointer_cast<dev::MaterialAsset>(selectedAsset));
    }

    DrawViewport();

    dev::ImGuiManager::Get().Render();
}

void SceneTestApp::DrawSceneTree()
{
    const auto isRootNode = [](dev::Entity entity)
    {
        if(entity.HasComponent<dev::HierarchyComponent>())
            return entity.GetComponent<dev::HierarchyComponent>().parent == entt::null;
        
        return true;
    };

    ImGui::Begin("Scene");
    
    auto pos = ImGui::GetCursorPos();

    ImGui::Dummy(ImGui::GetContentRegionAvail());

    if(ImGui::BeginDragDropTarget())
    {
        if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE"))
        {
            sceneAsset = *(dev::SceneAssetPtr*)payload->Data;

            scene = sceneAsset->scene;
            scene->SetRenderer(deferredRenderer);

            UpdateList();

            if(editorCamera)
                UpdateEditorCameraScript();
            else
                CreateEditorCameraEntity();
        }

        ImGui::EndDragDropTarget();
    }

    ImGui::SetCursorPos(pos);

    for(auto& entity : list)
    {
        if(entity.HasComponent<dev::CameraComponent>())
            entity.GetComponent<dev::CameraComponent>().active = false;

        if(isRootNode(entity))
            DrawEntityNode(entity);
    }

    if(ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
        selectedEntity = { entt::null, scene.get() };

    if(ImGui::BeginPopupContextWindow("Create entity"))
    {
        if(ImGui::MenuItem("Create Empty Entity"))
        {
            auto entity = scene->CreateEntity();

            entity.AddComponent<dev::NameComponent>().name = "Empty";

            selectedEntity = entity;

            list.push_back(entity);
        }

        if(ImGui::MenuItem("Create Drawable Entity"))
        {
            auto entity = scene->CreateEntity();

            entity.AddComponent<dev::NameComponent>().name = "Drawable";
            entity.AddComponent<dev::TransformComponent>();
            entity.AddComponent<dev::MeshComponent>();
            entity.AddComponent<dev::MeshRendererComponent>();
            entity.AddComponent<dev::PipelineComponent>(
                dev::AssetManager::Get().Load<dev::VertexShaderAsset>("vertex.vert", true),
                dev::AssetManager::Get().Load<dev::FragmentShaderAsset>("deferred.frag", true)
            );

            selectedEntity = entity;

            list.push_back(entity);
        }

        ImGui::EndPopup();
    }
    
    ImGui::End();
}

void SceneTestApp::DrawEntityNode(dev::Entity entity)
{
    bool hasChildren = false;
    if(entity.HasComponent<dev::HierarchyComponent>())
        hasChildren = !entity.GetComponent<dev::HierarchyComponent>().children.empty();

    std::string name = entity.HasComponent<dev::NameComponent>() 
                      ? entity.GetComponent<dev::NameComponent>().name 
                      : "Entity";

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    if(selectedEntity == entity)
        flags |= ImGuiTreeNodeFlags_Selected;
    if(!hasChildren)
        flags |= ImGuiTreeNodeFlags_Leaf;

    ImGui::PushID((int)(entt::entity)entity);
    bool isOpen = ImGui::TreeNodeEx(name.c_str(), flags);

    EntityNodeInteraction(entity, name);
    
    if(isOpen)
    {
        if(hasChildren)
        {
            auto& hierarchy = entity.GetComponent<dev::HierarchyComponent>();

            for(auto& child : hierarchy.children)
                DrawEntityNode(scene->GetEntity((entt::id_type)child));
        }
        ImGui::TreePop();
    }
    
    ImGui::PopID();
}

void SceneTestApp::EntityNodeInteraction(dev::Entity entity, std::string_view name)
{
    if(ImGui::IsItemClicked())
        selectedEntity = entity;

    if(ImGui::BeginDragDropTarget())
    {
        if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
        {
            auto payloadEntity = *(dev::Entity*)payload->Data;
            payloadEntity = scene->GetEntity((entt::id_type)(entt::entity)payloadEntity);

            if(entity != payloadEntity)
                scene->ReparentEntity(payloadEntity, entity);
        }

        ImGui::EndDragDropTarget();
    }

    if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::Text("Entity: %s", name.data());

        ImGui::SetDragDropPayload("ENTITY", &entity, 8);

        ImGui::EndDragDropSource();
    }
}

void SceneTestApp::DrawPropertiesWindow()
{
    ImGui::Begin("Properties");

    if(selectedEntity)
    {
        if(selectedEntity.HasComponent<dev::CameraComponent>())
            selectedEntity.GetComponent<dev::CameraComponent>().active = true;

        dev::DrawEntityUI<
            dev::NameComponent,
            dev::TransformComponent,
            dev::MeshComponent,
            dev::MeshRendererComponent,
            dev::CameraComponent,
            dev::LightComponent,
            dev::ScriptComponent,
            dev::TonemapComponent,
            dev::BloomComponent,
            dev::GTAOComponent,
            dev::SSRComponent,
            dev::ProceduralSkyComponent,
            dev::HDRISkyComponent,
            dev::RigidBodyComponent
        >(scene->GetRegistry(), selectedEntity);

        ImGui::Separator();

        if(ImGui::Button("Remove entity"))
        {
            auto it = std::find(list.begin(), list.end(), (entt::entity)selectedEntity);
            if (it != list.end())
                list.erase(it);

            scene->RemoveEntity(selectedEntity);

            selectedEntity = { entt::null, scene.get() };
        }

        if(ImGui::BeginPopupContextWindow("Add component"))
        {
            if(ImGui::MenuItem("Add NameComponent"))
                selectedEntity.GetOrAddComponent<dev::NameComponent>();

            if(ImGui::MenuItem("Add TransformComponent"))
                selectedEntity.GetOrAddComponent<dev::TransformComponent>();
            
            if(ImGui::MenuItem("Add MeshComponent"))
                selectedEntity.GetOrAddComponent<dev::MeshComponent>();
            
            if(ImGui::MenuItem("Add MeshRendererComponent"))
                selectedEntity.GetOrAddComponent<dev::MeshRendererComponent>();

            if(ImGui::MenuItem("Add PipelineComponent"))
                selectedEntity.GetOrAddComponent<dev::PipelineComponent>(
                    dev::AssetManager::Get().Load<dev::VertexShaderAsset>("vertex.vert", true),
                    dev::AssetManager::Get().Load<dev::FragmentShaderAsset>("deferred.frag", true)
                );
            
            if(ImGui::MenuItem("Add CameraComponent"))
            {
                auto& cameraComponent = selectedEntity.GetOrAddComponent<dev::CameraComponent>();
                cameraComponent.camera.SetPerspective();
                cameraComponent.camera.SetViewport(dev::Renderer::Get().GetViewportResolution());
            }
            
            if(ImGui::MenuItem("Add LightComponent"))
                selectedEntity.GetOrAddComponent<dev::LightComponent>();

            if(ImGui::MenuItem("Add RigidBodyComponent"))
                selectedEntity.GetOrAddComponent<dev::RigidBodyComponent>().body = 
                    dev::PhysicsManager::Get().CreateBody(
                        JPH::BodyCreationSettings(
                        new JPH::EmptyShapeSettings(),
                        { 0.0f, 0.0f, 0.0f },
                        { 0.0f, 0.0f, 0.0f, 1.0f },
                        JPH::EMotionType::Dynamic,
                        dev::Layers::moving
                        )
                    );
            
            if(ImGui::MenuItem("Add ScriptComponent"))
                selectedEntity.GetOrAddComponent<dev::ScriptComponent>();
            
            if(ImGui::MenuItem("Add TonemapComponent"))
                selectedEntity.GetOrAddComponent<dev::TonemapComponent>(LLGL::Extent2D{ 1280, 720 });
            
            if(ImGui::MenuItem("Add BloomComponent"))
                selectedEntity.GetOrAddComponent<dev::BloomComponent>(LLGL::Extent2D{ 1280, 720 });
            
            if(ImGui::MenuItem("Add GTAOComponent"))
                selectedEntity.GetOrAddComponent<dev::GTAOComponent>(LLGL::Extent2D{ 1280, 720 });
            
            if(ImGui::MenuItem("Add SSRComponent"))
                selectedEntity.GetOrAddComponent<dev::SSRComponent>(LLGL::Extent2D{ 1280, 720 });
            
            if(ImGui::MenuItem("Add ProceduralSkyComponent"))
                selectedEntity.GetOrAddComponent<dev::ProceduralSkyComponent>(LLGL::Extent2D{ 1024, 1024 });
            
            if(ImGui::MenuItem("Add HDRISkyComponent"))
                selectedEntity.GetOrAddComponent<dev::HDRISkyComponent>(
                    dev::AssetManager::Get().Load<dev::TextureAsset>("empty", true),
                    LLGL::Extent2D{ 1024, 1024 }
                );
            
            ImGui::EndPopup();
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

            auto& transform = selectedEntity.GetComponent<dev::TransformComponent>();
            auto modelMatrix = scene->GetWorldTransform(selectedEntity);
            glm::mat4 deltaMatrix(1.0f);

            ImGuizmo::SetDrawlist();
            
            ImGuizmo::SetRect(
                ImGui::GetWindowPos().x,
                ImGui::GetWindowPos().y + ImGui::GetFrameHeight(),
                ImGui::GetWindowWidth(),
                ImGui::GetWindowHeight() - ImGui::GetFrameHeight()
            );

            // Delta just skyrockets when trying to move an object with somewhat rotated parent...
            ImGuizmo::Manipulate(
                glm::value_ptr(viewMatrix),
                glm::value_ptr(projectionMatrix),
                currentOperation,
                ImGuizmo::MODE::WORLD,
                glm::value_ptr(modelMatrix),
                glm::value_ptr(deltaMatrix),
                dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::LeftControl) ? snap : nullptr
            );

            deltaMatrix[3] = glm::clamp(deltaMatrix[3], glm::vec4(-1.0f), glm::vec4(1.0f));

            transform.SetTransform(deltaMatrix * transform.GetTransform());

            if(selectedEntity.HasComponent<dev::RigidBodyComponent>())
            {
                auto body = selectedEntity.GetComponent<dev::RigidBodyComponent>().body;
                auto bodyId = body->GetID();

                auto pos = transform.position;
                auto rot = glm::quat(glm::radians(transform.rotation));
                auto scale = transform.scale;

                dev::PhysicsManager::Get().GetBodyInterface().SetPositionAndRotation(
                    bodyId,
                    { pos.x, pos.y, pos.z },
                    { rot.x, rot.y, rot.z, rot.w },
                    JPH::EActivation::Activate
                );
            }

            ImGuizmo::Enable(true);
        }
    }
    else
        ImGuizmo::Enable(false);
}

void SceneTestApp::DrawExecutionControl()
{
    ImGui::Begin("Execution Control");

    if(playing && !paused)
    {
        ImGui::BeginDisabled();
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    }

    if(ImGui::ImageButton("##Play", playIcon->nativeHandle, { 20, 20 }))
    {
        // Save scene state
        if(!paused)
            scene->Start();

        scene->SetIsRunning(true);

        playing = true;
        paused = false;
    }
    else if(playing && !paused)
    {
        ImGui::PopStyleVar();
        ImGui::EndDisabled();
    }

    ImGui::SameLine();

    if(!playing || paused)
    {
        ImGui::BeginDisabled();
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    }

    if(ImGui::ImageButton("##Pause", pauseIcon->nativeHandle, { 20, 20 }))
    {
        paused = true;
        scene->SetIsRunning(false);
    }
    else if(!playing || paused)
    {
        ImGui::PopStyleVar();
        ImGui::EndDisabled();
    }

    ImGui::SameLine();

    if(!playing && !paused)
    {
        ImGui::BeginDisabled();
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    }
    
    if(ImGui::ImageButton("##Stop", stopIcon->nativeHandle, { 20, 20 }))
    {
        playing = false;
        paused = false;

        scene->SetIsRunning(false);
        // Restore scene state
    }
    else if(!playing && !paused)
    {
        ImGui::PopStyleVar();
        ImGui::EndDisabled();
    }

    ImGui::SameLine();

    if(ImGui::ImageButton("##Build", buildIcon->nativeHandle, { 20, 20 }))
        dev::ScriptManager::Get().Build();

    ImGui::End();
}

void SceneTestApp::DrawMaterialPreview(dev::MaterialAssetPtr material, const ImVec2& size)
{
    if(material->albedo.type == dev::MaterialAsset::Property::Type::Texture)
    {
        if(ImGui::ImageButton("##Asset", material->albedo.texture->nativeHandle, size))
            selectedAsset = material;
    }
    else
    {
        ImVec4 color = ImVec4(material->albedo.value.x, material->albedo.value.y, material->albedo.value.z, material->albedo.value.w);
        
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
        
        if(ImGui::Button("##Asset", size))
            selectedAsset = material;

        ImGui::PopStyleColor(3); 
    }
}

// Kinda messy, clean it up
void SceneTestApp::DrawAssetBrowser()
{
    static const auto assetsPath = dev::AssetManager::Get().GetAssetsDirectory();
    auto selectedAssetPath = assetsPath;

    static auto currentDirectory = assetsPath;
    static std::string filter;
    
    static const auto& assets = dev::AssetManager::Get().GetAssets();

    ImGui::Begin("Assets");

    float regionWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
    int cols = std::max(1, (int)(regionWidth / 128.0f));

    ImGui::Text("%s", currentDirectory.c_str());

    ImGui::SameLine();
    
    if(ImGui::Button("..") && currentDirectory.has_parent_path())
        currentDirectory = currentDirectory.parent_path();

    ImGui::Separator();

    ImGui::InputText("Filter", &filter);

    ImGui::Separator();

    ImGui::Columns(cols, nullptr, false);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

    for(const auto& entry : std::filesystem::directory_iterator(currentDirectory))
    {
        ImGui::PushID(entry.path().c_str());

        if(entry.is_directory())
        {
            if(filter.empty() || entry.path().filename().string().find(filter) != std::string::npos)
            {
                if(ImGui::ImageButton("##Directory", folderIcon->nativeHandle, ImVec2(128.0f, 128.0f)))
                    currentDirectory = entry.path();

                ImGui::Text("%s", entry.path().filename().string().c_str());

                ImGui::NextColumn();
            }
        }
        else
        {
            if(filter.empty() || entry.path().filename().string().find(filter) != std::string::npos)
            {
                auto it = assets.find(entry.path());
                if(it != assets.end())
                    DrawAsset(entry.path(), it->second.second);
                else
                    DrawUnloadedAsset(entry.path());

                ImGui::Text("%s", entry.path().filename().string().c_str());

                ImGui::NextColumn();
            }
        }

        ImGui::PopID();
    }

    ImGui::Columns();

    ImGui::PopStyleVar();

    DrawCreateAssetMenu(currentDirectory);

    ImGui::End();
}

void SceneTestApp::DrawAsset(const std::filesystem::path& entry, dev::AssetPtr asset)
{
    switch(asset->type)
    {
        case dev::Asset::Type::Texture:
        {
            auto texture = std::dynamic_pointer_cast<dev::TextureAsset>(asset);
            
            ImGui::ImageButton("##Asset", texture->nativeHandle, ImVec2(128.0f, 128.0f));

            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                dev::TextureAssetPtr* payload = &texture;
                
                ImGui::SetDragDropPayload("TEXTURE", payload, 8);
                ImGui::Image(texture->nativeHandle, ImVec2(64,64));
                ImGui::Text("Texture: %s", entry.filename().string().c_str());

                ImGui::EndDragDropSource();
            }

            break;
        }

        case dev::Asset::Type::Material:
        {
            auto material = std::dynamic_pointer_cast<dev::MaterialAsset>(asset);
            
            DrawMaterialPreview(material, { 128.0f, 128.0f });

            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                dev::MaterialAssetPtr* payload = &material;
                
                ImGui::SetDragDropPayload("MATERIAL", payload, 8);
                DrawMaterialPreview(material, { 64.0f, 64.0f });
                ImGui::Text("Material: %s", entry.filename().string().c_str());

                ImGui::EndDragDropSource();
            }

            break;
        }

        case dev::Asset::Type::Model:
        {
            auto model = std::dynamic_pointer_cast<dev::ModelAsset>(asset);
            
            ImGui::ImageButton("##Asset", modelIcon->nativeHandle, ImVec2(128.0f, 128.0f));

            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                dev::ModelAssetPtr* payload = &model;
                
                ImGui::SetDragDropPayload("MODEL", payload, 8);
                ImGui::Image(modelIcon->nativeHandle, ImVec2(64,64));
                ImGui::Text("Model: %s", entry.filename().string().c_str());

                ImGui::EndDragDropSource();
            }

            break;
        }

        case dev::Asset::Type::Script:
        {
            auto script = std::dynamic_pointer_cast<dev::ScriptAsset>(asset);
            
            ImGui::ImageButton("##Asset", scriptIcon->nativeHandle, ImVec2(128.0f, 128.0f));

            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                dev::ScriptAssetPtr* payload = &script;
                
                ImGui::SetDragDropPayload("SCRIPT", payload, 8);
                ImGui::Image(scriptIcon->nativeHandle, ImVec2(64,64));
                ImGui::Text("Script: %s", entry.filename().string().c_str());

                ImGui::EndDragDropSource();
            }

            break;
        }

        case dev::Asset::Type::Scene:
        {
            auto scene = std::dynamic_pointer_cast<dev::SceneAsset>(asset);

            ImGui::ImageButton("##Asset", sceneIcon->nativeHandle, ImVec2(128.0f, 128.0f));

            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                dev::SceneAssetPtr* payload = &scene;
                
                ImGui::SetDragDropPayload("SCENE", payload, 8);
                ImGui::Image(scriptIcon->nativeHandle, ImVec2(64,64));
                ImGui::Text("Scene: %s", entry.filename().string().c_str());

                ImGui::EndDragDropSource();
            }

            break;
        }

        default:
            break;
    }
}

void SceneTestApp::DrawUnloadedAsset(const std::filesystem::path& entry)
{
    auto assetType = dev::GetAssetType(entry.extension().string());

    ImGui::ImageButton("##Asset", assetIcons[assetType]->nativeHandle, ImVec2(128.0f, 128.0f));

    if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
    {
        switch(assetType)
        {
            case dev::Asset::Type::Texture:
                dev::AssetManager::Get().Load<dev::TextureAsset>(entry);
                break;

            case dev::Asset::Type::Material:
                dev::AssetManager::Get().Load<dev::MaterialAsset>(entry);
                break;

            case dev::Asset::Type::Model:
                dev::AssetManager::Get().Load<dev::ModelAsset>(entry);
                break;

            case dev::Asset::Type::Script:
                dev::AssetManager::Get().Load<dev::ScriptAsset>(entry);
                break;

            case dev::Asset::Type::Scene:
                dev::AssetManager::Get().Load<dev::SceneAsset>(entry);
                break;

            default:
                break;
        }
    }
}

void SceneTestApp::DrawCreateAssetMenu(const std::filesystem::path& currentDirectory)
{
    static bool materialActive = false;
    static bool scriptActive = false;
    static bool sceneActive = false;

    if(ImGui::BeginPopupContextWindow("Create asset"))
    {
        materialActive = ImGui::MenuItem("Create material");
        scriptActive = ImGui::MenuItem("Create script");
        sceneActive = ImGui::MenuItem("Create scene");

        ImGui::EndPopup();
    }

    if(materialActive)
    {
        ImGui::OpenPopup("Create material");
        materialActive = DrawCreateMaterialMenu(currentDirectory);
    }
    else if(scriptActive)
    {
        ImGui::OpenPopup("Create script");
        scriptActive = DrawCreateScriptMenu(currentDirectory);
    }
    else if(sceneActive)
    {
        ImGui::OpenPopup("Create scene");
        sceneActive = DrawCreateSceneMenu(currentDirectory);
    }
}

bool SceneTestApp::DrawCreateMaterialMenu(const std::filesystem::path& currentDirectory)
{
    static std::string newMaterialName = "material";
    static int uniqueId = 0;
    bool active = true;

    if(ImGui::BeginPopupModal("Create material", &active, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("Material name", &newMaterialName);

        if(ImGui::Button("OK", ImVec2(120, 0)))
        {
            auto newMaterial = dev::AssetManager::Get().Load<dev::MaterialAsset>(currentDirectory / newMaterialName);
            
            newMaterialName = "material" + std::to_string(uniqueId++);

            ImGui::CloseCurrentPopup();

            active = false;
        }

        ImGui::SameLine();

        if(ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();

            active = false;
        }
        
        ImGui::EndPopup();
    }

    return active;
}

bool SceneTestApp::DrawCreateScriptMenu(const std::filesystem::path& currentDirectory)
{
    static std::string newScriptName = "script.as";
    static int uniqueId = 0;
    bool active = true;

    if(ImGui::BeginPopupModal("Create script", &active, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("Script name", &newScriptName);

        if(ImGui::Button("OK", ImVec2(120, 0)))
        {
            std::ofstream scriptFile(currentDirectory / newScriptName);
            scriptFile << 
                "void Start(Scene@ scene, Entity entity)\n"
                "{\n"
                "    \n"
                "}\n\n"
                "void Update(Scene@ scene, Entity entity, float deltaTime)\n"
                "{\n"
                "    \n"
                "}\n";
            scriptFile.close();
            system(std::string("code " + currentDirectory.string() + "/" + newScriptName).c_str());

            auto newScript = dev::AssetManager::Get().Load<dev::ScriptAsset>(currentDirectory / newScriptName);
            
            newScriptName = "script" + std::to_string(uniqueId++) + ".as";

            ImGui::CloseCurrentPopup();

            active = false;
        }

        ImGui::SameLine();

        if(ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();

            active = false;
        }
        
        ImGui::EndPopup();
    }

    return active;
}

bool SceneTestApp::DrawCreateSceneMenu(const std::filesystem::path& currentDirectory)
{
    static std::string newSceneName = "scene.json";
    static int uniqueId = 0;
    bool active = true;

    if(ImGui::BeginPopupModal("Create scene", &active, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("Scene name", &newSceneName);

        if(ImGui::Button("OK", ImVec2(120, 0)))
        {
            dev::SceneAssetPtr newScene = 
                std::make_shared<dev::SceneAsset>(
                    std::make_shared<dev::Scene>(deferredRenderer)
                );

            newScene->path = currentDirectory / newSceneName;

            dev::AssetManager::Get().Write(newScene);
            
            newSceneName = "scene" + std::to_string(uniqueId++) + ".json";

            ImGui::CloseCurrentPopup();

            active = false;
        }

        ImGui::SameLine();

        if(ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();

            active = false;
        }
        
        ImGui::EndPopup();
    }

    return active;
}

void SceneTestApp::DrawMaterialEditor(dev::MaterialAssetPtr material)
{
    static dev::Timer saveTimer;

    ImGui::Begin("Material Editor");

    if((ImGui::Button("Save") || saveTimer.GetElapsedSeconds() > 3.0f) && ImGui::IsWindowFocused())
    {
        dev::AssetManager::Get().Write(material);

        saveTimer.Reset();
    }

    ImGui::Separator();
    
    ImGui::Text("Albedo:");
    DrawMaterialProperty(material->albedo, 1);

    ImGui::Text("Normal:");
    DrawMaterialProperty(material->normal, 2);

    ImGui::Text("Metallic:");
    DrawMaterialProperty(material->metallic, 3, true);

    ImGui::Text("Roughness:");
    DrawMaterialProperty(material->roughness, 4, true);

    ImGui::Text("Ambient occlusion:");
    DrawMaterialProperty(material->ao, 5);

    ImGui::Text("Emission:");
    DrawMaterialProperty(material->emission, 6);

    ImGui::Text("Emission strength:");
    ImGui::DragFloat("##EmissionStrength", &material->emissionStrength, 0.01f, 0.0f, 100.0f);

    ImGui::Separator();
    ImGui::Text("UV Scale:");
    ImGui::DragFloat2("##UVScale", &material->uvScale.x, 0.01f, 0.0f, 100.0f);

    ImGui::End();
}

void SceneTestApp::DrawMaterialProperty(dev::MaterialAsset::Property& property, int id, bool singleComponent)
{
    ImGui::PushID(id);

    if(property.type == dev::MaterialAsset::Property::Type::Color)
    {
        if(singleComponent)
            ImGui::DragFloat("##Value", &property.value.x, 0.01f, 0.0f, 1.0f);
        else
            ImGui::ColorEdit4("##Color", &property.value.x);

        if(ImGui::Button("Set Texture"))
            property.type = dev::MaterialAsset::Property::Type::Texture;
    }
    else
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        ImGui::Image(property.texture->nativeHandle, ImVec2(128.0f, 128.0f));

        ImGui::PopStyleVar();
        
        if(ImGui::BeginDragDropTarget())
        {
            auto payload = ImGui::AcceptDragDropPayload("TEXTURE");
            
            if(payload)
                property.texture = *(dev::TextureAssetPtr*)payload->Data;

            ImGui::EndDragDropTarget();
        }

        if(ImGui::Button("Set Color"))
            property.type = dev::MaterialAsset::Property::Type::Color;
    }

    ImGui::PopID();

    ImGui::Separator();
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
        eventTimer.GetElapsedSeconds() > 0.02f)
    {
        dev::Multithreading::Get().AddJob({ {}, [size]()
        {
            dev::Renderer::Get().SetViewportResolution({ (uint32_t)size.x, (uint32_t)size.y  });

            dev::EventManager::Get().Dispatch(
                std::make_unique<dev::WindowResizeEvent>(
                    LLGL::Extent2D{ (uint32_t)size.x, (uint32_t)size.y }
                )
            );
        } });

        eventTimer.Reset();
    }

    ImGui::Image(nativeViewportAttachment, window->InnerRect.GetSize());

    if(ImGui::BeginDragDropTarget())
    {
        auto payload = ImGui::AcceptDragDropPayload("MODEL");

        if(payload)
            CreateModelEntity(*(dev::ModelAssetPtr*)payload->Data, true);

        ImGui::EndDragDropTarget();
    }

    canMoveCamera = ImGui::IsWindowHovered();

    DrawImGuizmo();

    auto lights = scene->GetRegistry().view<dev::TransformComponent, dev::LightComponent>();

    for(auto entity : lights)
    {
        auto[transform, light] = 
            lights.get<dev::TransformComponent, dev::LightComponent>(entity);

        /* auto screenPos = camera.GetComponent<dev::CameraComponent>().camera.WorldToScreen(transform.position);

        if(!glm::any(glm::isnan(screenPos)))
        {
            ImGui::PushID((int)entity);

            ImGui::SetCursorPos({ screenPos.x - 32.0f, screenPos.y - 36.0f });

            ImGui::Image(
                lightIcon->nativeHandle,
                { 64, 64 },
                { 0, 0 },
                { 1, 1 },
                { light.color.x, light.color.y, light.color.z, 1.0f }
            );

            ImGui::PopID();
        } */
    }

    ImGui::End();

    ImGui::PopStyleVar();
}

void SceneTestApp::Draw()
{
    if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::G) && keyboardTimer.GetElapsedSeconds() > 0.2f)
    {
        scene->ToggleUpdatePhysics();
        keyboardTimer.Reset();
    }

    scene->Draw(viewportRenderTarget);

    dev::Renderer::Get().ClearRenderTarget();

    DrawImGui();

    dev::Renderer::Get().Present();
}

void SceneTestApp::OnEvent(dev::Event& event)
{
    if(event.GetType() == dev::Event::Type::WindowResize)
    {
        auto resizeEvent = dynamic_cast<dev::WindowResizeEvent*>(&event);

        CreateRenderTarget(resizeEvent->GetSize());
    }
}

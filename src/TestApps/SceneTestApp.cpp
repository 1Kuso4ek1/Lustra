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

    dev::PhysicsManager::Get().Init();

    dev::EventManager::Get().AddListener(dev::Event::Type::WindowResize, this);

    SetupAssetManager();

    LoadShaders();
    LoadTextures();

    deferredRenderer = std::make_shared<dev::DeferredRenderer>();

    scene.SetRenderer(deferredRenderer);

    CreateRenderTarget();

    CreateEntities();

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
    dev::AssetManager::Get().AddLoader<dev::MaterialAsset, dev::MaterialLoader>("materials");
    dev::AssetManager::Get().AddLoader<dev::ModelAsset, dev::ModelLoader>("models");
}

// TODO: Make ShaderAsset
void SceneTestApp::LoadShaders()
{
    vertexShader = dev::Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/vertex.vert");
    fragmentShader = dev::Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/deferred.frag");

    pipeline = dev::Renderer::Get().CreatePipelineState(vertexShader, fragmentShader);
}

void SceneTestApp::LoadTextures()
{
    texture = dev::AssetManager::Get().Load<dev::TextureAsset>("tex.jpg", true);

    metal = dev::AssetManager::Get().Load<dev::TextureAsset>("Metall_ak-47_Base_Color.png", true);
    wood = dev::AssetManager::Get().Load<dev::TextureAsset>("Wood_ak-47_Base_Color.png", true);

    ak47Metal = dev::AssetManager::Get().Load<dev::MaterialAsset>("ak47Metal", true);
    ak47Wood = dev::AssetManager::Get().Load<dev::MaterialAsset>("ak47Wood", true);

    ak47Metal->albedo = metal;
    ak47Wood->albedo = wood;

    fileIcon = dev::AssetManager::Get().Load<dev::TextureAsset>("icons/file.png", true);
    folderIcon = dev::AssetManager::Get().Load<dev::TextureAsset>("icons/folder.png", true);
    textureIcon = dev::AssetManager::Get().Load<dev::TextureAsset>("icons/texture.png", true);
    materialIcon = dev::AssetManager::Get().Load<dev::TextureAsset>("icons/material.png", true);
    modelIcon = dev::AssetManager::Get().Load<dev::TextureAsset>("icons/model.png", true);

    assetIcons = 
    {
        { dev::Asset::Type::Texture, textureIcon },
        { dev::Asset::Type::Material, materialIcon },
        { dev::Asset::Type::Model, modelIcon },
        { dev::Asset::Type::Unknown, fileIcon }
    };
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
    rifle.AddComponent<dev::MeshComponent>().model = dev::AssetManager::Get().Load<dev::ModelAsset>("ak-47.fbx", true);
    rifle.AddComponent<dev::MeshRendererComponent>().materials = { ak47Wood, ak47Metal };
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
                speed = lerp(speed, 3.0f, lerpSpeed);
                movement = input;
            }
            else
                speed = lerp(speed, 0.0f, lerpSpeed);

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
    postProcessing.AddComponent<dev::TonemapComponent>(LLGL::Extent2D{ 1280, 720 });
    postProcessing.AddComponent<dev::BloomComponent>(LLGL::Extent2D{ 1280, 720 });
    postProcessing.AddComponent<dev::GTAOComponent>(LLGL::Extent2D{ 1280, 720 });
    postProcessing.AddComponent<dev::SSRComponent>(LLGL::Extent2D{ 1280, 720 });
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
    sky.AddComponent<dev::MeshComponent>().model = dev::AssetManager::Get().Load<dev::ModelAsset>("cube", true);

    //sky.AddComponent<dev::ProceduralSkyComponent>(LLGL::Extent2D{ 1024, 1024 });
    sky.AddComponent<dev::HDRISkyComponent>(
        dev::AssetManager::Get().Load<dev::TextureAsset>("hdri/meadow_2_1k.hdr", true),
        LLGL::Extent2D{ 1024, 1024 }
    );
}

void SceneTestApp::CreateModelEntity(dev::ModelAssetPtr model, bool relativeToCamera)
{
    auto entity = scene.CreateEntity();

    entity.AddComponent<dev::NameComponent>().name = "Model";
    entity.AddComponent<dev::TransformComponent>();
    entity.AddComponent<dev::MeshComponent>().model = model;
    entity.AddComponent<dev::MeshRendererComponent>();
    entity.AddComponent<dev::PipelineComponent>().pipeline = pipeline;
    
    auto& rigidBody = entity.AddComponent<dev::RigidBodyComponent>();

    auto& modelPos = entity.GetComponent<dev::TransformComponent>().position;

    if(relativeToCamera)
    {
        auto& cameraPos = camera.GetComponent<dev::TransformComponent>().position;

        auto cameraOrient = glm::quat(glm::radians(camera.GetComponent<dev::TransformComponent>().rotation));

        modelPos = cameraPos + cameraOrient * glm::vec3(0.0f, 0.0f, -5.0f);
    }

    rigidBody.body = dev::PhysicsManager::Get().CreateBody(
        JPH::BodyCreationSettings(
            new JPH::BoxShapeSettings({ 1.0f, 1.0f, 1.0f }),
            { modelPos.x, modelPos.y, modelPos.z },
            { 0.0f, 0.0f, 0.0f, 1.0f },
            JPH::EMotionType::Dynamic,
            dev::Layers::moving
        )
    );

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
    ImGui::Begin("Scene");

    for(auto entity : list)
    {
        std::string name = (entity.HasComponent<dev::NameComponent>() ? entity.GetComponent<dev::NameComponent>().name : "Entity");

        ImGui::PushID((entt::id_type)(entt::entity)entity);

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
        dev::DrawEntityUI<
            dev::NameComponent,
            dev::TransformComponent,
            dev::MeshRendererComponent,
            dev::CameraComponent,
            dev::LightComponent,
            dev::TonemapComponent,
            dev::BloomComponent,
            dev::GTAOComponent,
            dev::SSRComponent,
            dev::ProceduralSkyComponent,
            dev::HDRISkyComponent,
            dev::RigidBodyComponent
        >(scene.GetRegistry(), selectedEntity);

        ImGui::Separator();

        if(ImGui::Button("Remove entity"))
        {
            auto it = std::find(list.begin(), list.end(), (entt::entity)selectedEntity);
            if (it != list.end())
                list.erase(it);

            scene.RemoveEntity(selectedEntity);

            selectedEntity = { entt::null, &scene };
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
            auto modelMatrix = transform.GetTransform();

            ImGuizmo::SetDrawlist();
            
            ImGuizmo::SetRect(
                ImGui::GetWindowPos().x,
                ImGui::GetWindowPos().y,
                ImGui::GetWindowWidth(),
                ImGui::GetWindowHeight()
            );

            ImGuizmo::Manipulate(
                glm::value_ptr(viewMatrix),
                glm::value_ptr(projectionMatrix),
                currentOperation,
                ImGuizmo::MODE::WORLD,
                glm::value_ptr(modelMatrix),
                nullptr,
                dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::LeftControl) ? snap : nullptr
            );

            transform.SetTransform(modelMatrix);

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
    auto assetsPath = dev::AssetManager::Get().GetAssetsDirectory();
    auto selectedAssetPath = assetsPath;

    static auto currentDirectory = assetsPath;
    static std::string filter;
    
    auto assets = dev::AssetManager::Get().GetAssets();

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

            default:
                break;
        }
    }
}

void SceneTestApp::DrawCreateAssetMenu(const std::filesystem::path& currentDirectory)
{
    static std::string newMaterialName = "material";
    static int uniqueId = 0;

    static bool active = false;

    if(ImGui::BeginPopupContextWindow("Create asset"))
    {
        if(ImGui::MenuItem("Create material"))
            active = true;

        ImGui::EndPopup();
    }

    if(active)
        ImGui::OpenPopup("Create material");

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
}

void SceneTestApp::DrawMaterialEditor(dev::MaterialAssetPtr material)
{
    ImGui::Begin("Material Editor");
    
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

    ImGui::End();

    ImGui::PopStyleVar();
}

void SceneTestApp::Draw()
{
    if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::G) && keyboardTimer.GetElapsedSeconds() > 0.2f)
    {
        scene.ToggleUpdatePhysics();
        keyboardTimer.Reset();
    }

    scene.Draw(viewportRenderTarget);

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

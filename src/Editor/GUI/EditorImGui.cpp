#include <Editor.hpp>

void Editor::DrawImGui()
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

void Editor::DrawSceneTree()
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

            selectedEntity = {};

            dev::Multithreading::Get().AddJob({ {},
                [&]() {
                    scene = sceneAsset->scene;
                    scene->SetRenderer(deferredRenderer);

                    dev::EventManager::Get().Dispatch(
                        std::make_unique<dev::WindowResizeEvent>(dev::Renderer::Get().GetViewportResolution())
                    );

                    UpdateList();

                    if(editorCamera)
                        UpdateEditorCameraScript();
                    else
                        CreateEditorCameraEntity();
                }
            });
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

void Editor::DrawEntityNode(dev::Entity entity)
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

void Editor::EntityNodeInteraction(dev::Entity entity, std::string_view name)
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

void Editor::DrawPropertiesWindow()
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

void Editor::DrawImGuizmoControls()
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

void Editor::DrawImGuizmo()
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

void Editor::DrawExecutionControl()
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

void Editor::DrawViewport()
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

        auto screenPos = editorCamera.GetComponent<dev::CameraComponent>().camera.WorldToScreen(transform.position);

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
        }
    }

    ImGui::End();

    ImGui::PopStyleVar();
}

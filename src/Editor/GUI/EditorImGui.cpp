#include <Editor.hpp>

void Editor::RenderImGui()
{
    if(fullscreenViewport)
        return;

    lustra::Renderer::Get().ClearRenderTarget();

    lustra::ImGuiManager::Get().NewFrame();

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
        if(selectedAsset->type == lustra::Asset::Type::Material)
            DrawMaterialEditor(std::static_pointer_cast<lustra::MaterialAsset>(selectedAsset));
    }

    DrawViewport();

    lustra::ImGuiManager::Get().Render();
}

void Editor::DrawSceneTree()
{
    const auto isRootNode = [](lustra::Entity entity)
    {
        if(entity.HasComponent<lustra::HierarchyComponent>())
            return entity.GetComponent<lustra::HierarchyComponent>().parent == entt::null;

        return true;
    };

    ImGui::Begin("Scene");

    const auto pos = ImGui::GetCursorPos();

    ImGui::Dummy(ImGui::GetContentRegionAvail());

    if(ImGui::BeginDragDropTarget())
    {
        if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE"))
        {
            sceneAsset = *static_cast<lustra::SceneAssetPtr*>(payload->Data);

            selectedEntity = {};

            lustra::Multithreading::Get().AddJob({ {},
                [&] { SwitchScene(sceneAsset); }
            });
        }

        ImGui::EndDragDropTarget();
    }

    ImGui::SetCursorPos(pos);

    for(auto& entity : list)
    {
        if(entity.HasComponent<lustra::CameraComponent>())
            entity.GetComponent<lustra::CameraComponent>().active = false;

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

            entity.AddComponent<lustra::NameComponent>().name = "Empty";

            selectedEntity = entity;

            list.push_back(entity);
        }

        if(ImGui::MenuItem("Create Drawable Entity"))
        {
            auto entity = scene->CreateEntity();

            entity.AddComponent<lustra::NameComponent>().name = "Drawable";
            entity.AddComponent<lustra::TransformComponent>();
            entity.AddComponent<lustra::MeshComponent>();
            entity.AddComponent<lustra::MeshRendererComponent>();
            entity.AddComponent<lustra::PipelineComponent>(
                lustra::AssetManager::Get().Load<lustra::VertexShaderAsset>("vertex.vert", true),
                lustra::AssetManager::Get().Load<lustra::FragmentShaderAsset>("deferred.frag", true)
            );

            selectedEntity = entity;

            list.push_back(entity);
        }

        ImGui::EndPopup();
    }

    if(ImGui::IsWindowFocused())
    {
        if(CheckShortcut({ lustra::Keyboard::Key::LeftControl, lustra::Keyboard::Key::C }))
        {
            buffer = selectedEntity;
            keyboardTimer.Reset();
        }

        if(CheckShortcut({ lustra::Keyboard::Key::LeftControl, lustra::Keyboard::Key::V }))
        {
            if(buffer)
                list.push_back(selectedEntity = scene->CloneEntity(buffer));

            keyboardTimer.Reset();
        }
    }

    ImGui::End();
}

void Editor::DrawEntityNode(lustra::Entity entity)
{
    bool hasChildren = false;
    if(entity.HasComponent<lustra::HierarchyComponent>())
        hasChildren = !entity.GetComponent<lustra::HierarchyComponent>().children.empty();

    const auto name = entity.HasComponent<lustra::NameComponent>()
                      ? entity.GetComponent<lustra::NameComponent>().name
                      : "Entity";

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    if(selectedEntity == entity)
        flags |= ImGuiTreeNodeFlags_Selected;
    if(!hasChildren)
        flags |= ImGuiTreeNodeFlags_Leaf;

    ImGui::PushID(static_cast<int>(static_cast<entt::entity>(entity)));
    const bool isOpen = ImGui::TreeNodeEx(name.c_str(), flags);

    EntityNodeInteraction(entity, name);

    if(isOpen)
    {
        if(hasChildren)
        {
            auto& hierarchy = entity.GetComponent<lustra::HierarchyComponent>();

            for(auto& child : hierarchy.children)
                DrawEntityNode(scene->GetEntity(static_cast<entt::id_type>(child)));
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void Editor::EntityNodeInteraction(const lustra::Entity& entity, const std::string_view name)
{
    if(ImGui::IsItemClicked())
        selectedEntity = entity;

    if(ImGui::BeginDragDropTarget())
    {
        if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
        {
            auto payloadEntity = *static_cast<lustra::Entity *>(payload->Data);
            payloadEntity = scene->GetEntity(static_cast<entt::id_type>(static_cast<entt::entity>(payloadEntity)));

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
        if(selectedEntity.HasComponent<lustra::CameraComponent>())
            selectedEntity.GetComponent<lustra::CameraComponent>().active = true;

        lustra::DrawEntityUI<
            lustra::NameComponent,
            lustra::TransformComponent,
            lustra::MeshComponent,
            lustra::MeshRendererComponent,
            lustra::PipelineComponent,
            lustra::CameraComponent,
            lustra::LightComponent,
            lustra::ScriptComponent,
            lustra::PrefabComponent,
            lustra::TonemapComponent,
            lustra::BloomComponent,
            lustra::GTAOComponent,
            lustra::SSRComponent,
            lustra::ProceduralSkyComponent,
            lustra::HDRISkyComponent,
            lustra::SoundComponent,
            lustra::RigidBodyComponent
        >(scene->GetRegistry(), selectedEntity);

        ImGui::Separator();

        if(ImGui::Button("Remove entity"))
        {
            const auto it = std::ranges::find(list, static_cast<entt::entity>(selectedEntity));
            if(it != list.end())
                list.erase(it);

            scene->RemoveEntity(selectedEntity);

            selectedEntity = { entt::null, scene.get() };
        }

        if(ImGui::BeginPopupContextWindow("Add component"))
        {
            if(ImGui::MenuItem("Add NameComponent"))
                selectedEntity.GetOrAddComponent<lustra::NameComponent>();

            if(ImGui::MenuItem("Add TransformComponent"))
                selectedEntity.GetOrAddComponent<lustra::TransformComponent>();

            if(ImGui::MenuItem("Add MeshComponent"))
                selectedEntity.GetOrAddComponent<lustra::MeshComponent>();

            if(ImGui::MenuItem("Add MeshRendererComponent"))
                selectedEntity.GetOrAddComponent<lustra::MeshRendererComponent>();

            if(ImGui::MenuItem("Add PipelineComponent"))
                selectedEntity.GetOrAddComponent<lustra::PipelineComponent>(
                    lustra::AssetManager::Get().Load<lustra::VertexShaderAsset>("vertex.vert", true),
                    lustra::AssetManager::Get().Load<lustra::FragmentShaderAsset>("deferred.frag", true)
                );

            if(ImGui::MenuItem("Add CameraComponent"))
            {
                auto& cameraComponent = selectedEntity.GetOrAddComponent<lustra::CameraComponent>();
                cameraComponent.camera.SetPerspective();
                cameraComponent.camera.SetViewport(lustra::Renderer::Get().GetViewportResolution());
            }

            if(ImGui::MenuItem("Add LightComponent"))
                selectedEntity.GetOrAddComponent<lustra::LightComponent>();

            if(ImGui::MenuItem("Add RigidBodyComponent"))
                selectedEntity.GetOrAddComponent<lustra::RigidBodyComponent>().body =
                    lustra::PhysicsManager::Get().CreateBody(
                        JPH::BodyCreationSettings(
                            new JPH::EmptyShapeSettings(),
                            { 0.0f, 0.0f, 0.0f },
                            { 0.0f, 0.0f, 0.0f, 1.0f },
                            JPH::EMotionType::Dynamic,
                            lustra::Layers::moving
                        )
                    );

            if(ImGui::MenuItem("Add SoundComponent"))
                selectedEntity.GetOrAddComponent<lustra::SoundComponent>();

            if(ImGui::MenuItem("Add ScriptComponent"))
                selectedEntity.GetOrAddComponent<lustra::ScriptComponent>();

            if(ImGui::MenuItem("Add PrefabComponent"))
                selectedEntity.GetOrAddComponent<lustra::PrefabComponent>();

            if(ImGui::MenuItem("Add TonemapComponent"))
                selectedEntity.GetOrAddComponent<lustra::TonemapComponent>(LLGL::Extent2D{ 1280, 720 });

            if(ImGui::MenuItem("Add BloomComponent"))
                selectedEntity.GetOrAddComponent<lustra::BloomComponent>(LLGL::Extent2D{ 1280, 720 });

            if(ImGui::MenuItem("Add GTAOComponent"))
                selectedEntity.GetOrAddComponent<lustra::GTAOComponent>(LLGL::Extent2D{ 1280, 720 });

            if(ImGui::MenuItem("Add SSRComponent"))
                selectedEntity.GetOrAddComponent<lustra::SSRComponent>(LLGL::Extent2D{ 1280, 720 });

            if(ImGui::MenuItem("Add ProceduralSkyComponent"))
                selectedEntity.GetOrAddComponent<lustra::ProceduralSkyComponent>(LLGL::Extent2D{ 1024, 1024 });

            if(ImGui::MenuItem("Add HDRISkyComponent"))
                selectedEntity.GetOrAddComponent<lustra::HDRISkyComponent>(
                    lustra::AssetManager::Get().Load<lustra::TextureAsset>("empty", true),
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

    if(ImGui::RadioButton("Translate", currentOperation == ImGuizmo::OPERATION::TRANSLATE)
        || CheckShortcut({ lustra::Keyboard::Key::LeftAlt, lustra::Keyboard::Key::T }))
        currentOperation = ImGuizmo::OPERATION::TRANSLATE;

    ImGui::SameLine();
    if(ImGui::RadioButton("Rotate", currentOperation == ImGuizmo::OPERATION::ROTATE)
        || CheckShortcut({ lustra::Keyboard::Key::LeftAlt, lustra::Keyboard::Key::R }))
        currentOperation = ImGuizmo::OPERATION::ROTATE;

    ImGui::SameLine();
    if(ImGui::RadioButton("Scale", currentOperation == ImGuizmo::OPERATION::SCALE)
        || CheckShortcut({ lustra::Keyboard::Key::LeftAlt, lustra::Keyboard::Key::S }))
        currentOperation = ImGuizmo::OPERATION::SCALE;

    ImGui::DragFloat3("Snap", snap, 0.1f, 0.0f, 45.0f);

    ImGui::End();
}

void Editor::DrawImGuizmo()
{
    if(selectedEntity)
    {
        if(selectedEntity.HasComponent<lustra::TransformComponent>())
        {
            auto viewMatrix = lustra::Renderer::Get().GetMatrices()->GetView();
            auto projectionMatrix = lustra::Renderer::Get().GetMatrices()->GetProjection();

            auto& transform = selectedEntity.GetComponent<lustra::TransformComponent>();
            auto modelMatrix = scene->GetWorldTransform(selectedEntity);
            glm::mat4 deltaMatrix(1.0f);

            ImGuizmo::SetDrawlist();

            ImGuizmo::SetRect(
                ImGui::GetWindowPos().x,
                ImGui::GetWindowPos().y + ImGui::GetFrameHeight(),
                ImGui::GetWindowWidth(),
                ImGui::GetWindowHeight() - ImGui::GetFrameHeight()
            );

            // Delta just skyrockets when trying to move an object with a somewhat rotated parent...
            ImGuizmo::Manipulate(
                glm::value_ptr(viewMatrix),
                glm::value_ptr(projectionMatrix),
                currentOperation,
                ImGuizmo::MODE::WORLD,
                glm::value_ptr(modelMatrix),
                glm::value_ptr(deltaMatrix),
                lustra::Keyboard::IsKeyPressed(lustra::Keyboard::Key::LeftControl) ? snap : nullptr
            );

            deltaMatrix[3] = glm::clamp(deltaMatrix[3], glm::vec4(-1.0f), glm::vec4(1.0f));

            transform.SetTransform(deltaMatrix * transform.GetTransform());

            if(selectedEntity.HasComponent<lustra::RigidBodyComponent>())
            {
                const auto body = selectedEntity.GetComponent<lustra::RigidBodyComponent>().body;
                const auto bodyId = body->GetID();

                auto pos = transform.position;
                auto rot = glm::quat(glm::radians(transform.rotation));

                lustra::PhysicsManager::Get().GetBodyInterface().SetPositionAndRotation(
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
        {
            lustra::AssetManager::Get().Write(sceneAsset);

            scene->Start();
            scene->SetUpdatePhysics(true);
        }

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
        scene->GetRegistry().clear<>();
        selectedEntity = {};

        // Restore scene state
        lustra::Multithreading::Get().AddJob({ {},
            [&] {
                SwitchScene(
                    lustra::AssetManager::Get().Load<lustra::SceneAsset>(sceneAsset->path, false, false)
                );
            }
        });
    }
    else if(!playing && !paused)
    {
        ImGui::PopStyleVar();
        ImGui::EndDisabled();
    }

    ImGui::SameLine();

    if(ImGui::ImageButton("##Build", buildIcon->nativeHandle, { 20, 20 }))
        lustra::ScriptManager::Get().Build();

    ImGui::End();
}

void Editor::DrawLog()
{

}

void Editor::DrawViewport()
{
    static lustra::Timer eventTimer;
    static ImGuiWindowFlags flags = 0;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::Begin("Viewport", nullptr, flags);

    const auto window = ImGui::GetCurrentWindow();
    flags = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(window->InnerRect.Min, window->InnerRect.Max) ? ImGuiWindowFlags_NoMove : 0;

    auto size = window->InnerRect.GetSize();
    if((size.x != static_cast<float>(viewportRenderTarget->GetResolution().width) ||
        size.y != static_cast<float>(viewportRenderTarget->GetResolution().height)) &&
        eventTimer.GetElapsedSeconds() > 0.02f)
    {
        lustra::Multithreading::Get().AddJob({ {}, [size]
        {
            lustra::Renderer::Get().SetViewportResolution({ static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y)  });

            lustra::EventManager::Get().Dispatch(
                std::make_unique<lustra::WindowResizeEvent>(
                    LLGL::Extent2D{ static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) }
                )
            );
        } });

        eventTimer.Reset();
    }

    ImGui::Image(nativeViewportAttachment, window->InnerRect.GetSize());

    if(ImGui::BeginDragDropTarget())
    {
        if(const auto payload = ImGui::AcceptDragDropPayload("MODEL"))
            CreateModelEntity(*static_cast<lustra::ModelAssetPtr*>(payload->Data), true);

        ImGui::EndDragDropTarget();
    }

    canMoveCamera = ImGui::IsWindowHovered();

    DrawOnScreenIcons();

    DrawImGuizmo();

    ImGui::End();

    ImGui::PopStyleVar();
}

void Editor::DrawOnScreenIcons()
{
    auto lights = scene->GetRegistry().view<lustra::TransformComponent, lustra::LightComponent>();
    auto sounds = scene->GetRegistry().view<lustra::TransformComponent, lustra::SoundComponent>();
    auto cameras = scene->GetRegistry().view<lustra::TransformComponent, lustra::CameraComponent>();

    auto drawOnScreen = [&](
        const glm::vec3& pos,
        const int id,
        const lustra::TextureAssetPtr &icon,
        const glm::vec3& lightColor = glm::vec3(-1.0f))
    {
        const auto screenPos = editorCamera.GetComponent<lustra::CameraComponent>().camera.WorldToScreen(pos);

        if(!glm::any(glm::isnan(screenPos)))
        {
            ImGui::PushID(id);

            ImGui::SetCursorPos({ screenPos.x - 32.0f, screenPos.y - 36.0f });

            ImGui::Image(
                icon->nativeHandle,
                { 64, 64 },
                { 0, 0 },
                { 1, 1 },
                { lightColor.x, lightColor.y, lightColor.z, 0.5f },
                { 0.0f, 0.0f, 0.0f, 0.0f }
            );

            ImGui::PopID();
        }
    };

    if(!playing)
    {
        for(auto entity : lights)
        {
            auto& light = lights.get<lustra::LightComponent>(entity);

            lustra::TransformComponent transform;
            transform.SetTransform(scene->GetWorldTransform(entity));

            drawOnScreen(transform.position, static_cast<int>(entity), lightIcon, light.color);
        }

        for(auto entity : sounds)
        {
            lustra::TransformComponent transform;
            transform.SetTransform(scene->GetWorldTransform(entity));

            drawOnScreen(transform.position, static_cast<int>(entity), soundIcon);
        }

        for(auto entity : cameras)
        {
            const auto& camera = cameras.get<lustra::CameraComponent>(entity);

            if(entity == editorCamera || camera.active)
                continue;

            lustra::TransformComponent transform;
            transform.SetTransform(scene->GetWorldTransform(entity));

            drawOnScreen(transform.position, static_cast<int>(entity), cameraIcon);
        }
    }
}

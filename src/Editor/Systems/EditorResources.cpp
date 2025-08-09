#include <Editor.hpp>

void Editor::LoadIcons()
{
    const auto currentDir = lustra::AssetManager::Get().GetAssetsDirectory();

    lustra::AssetManager::Get().SetAssetsDirectory(EDITOR_ROOT / std::filesystem::path("resources"));

    const auto fileIcon = lustra::AssetManager::Get().Load<lustra::TextureAsset>("icons/file.png", true);
    const auto folderIcon = lustra::AssetManager::Get().Load<lustra::TextureAsset>("icons/folder.png", true);
    const auto textureIcon = lustra::AssetManager::Get().Load<lustra::TextureAsset>("icons/texture.png", true);
    const auto materialIcon = lustra::AssetManager::Get().Load<lustra::TextureAsset>("icons/material.png", true);
    const auto modelIcon = lustra::AssetManager::Get().Load<lustra::TextureAsset>("icons/model.png", true);
    const auto scriptIcon = lustra::AssetManager::Get().Load<lustra::TextureAsset>("icons/script.png", true);
    const auto sceneIcon = lustra::AssetManager::Get().Load<lustra::TextureAsset>("icons/scene.png", true);

    const auto playIcon = lustra::AssetManager::Get().Load<lustra::TextureAsset>("icons/play.png", true);
    const auto pauseIcon = lustra::AssetManager::Get().Load<lustra::TextureAsset>("icons/pause.png", true);
    const auto stopIcon = lustra::AssetManager::Get().Load<lustra::TextureAsset>("icons/stop.png", true);
    const auto buildIcon = lustra::AssetManager::Get().Load<lustra::TextureAsset>("icons/build.png", true);

    const auto lightIcon = lustra::AssetManager::Get().Load<lustra::TextureAsset>("icons/light.png", true);
    const auto soundIcon = lustra::AssetManager::Get().Load<lustra::TextureAsset>("icons/sound.png", true);
    const auto cameraIcon = lustra::AssetManager::Get().Load<lustra::TextureAsset>("icons/camera.png", true);

    const auto vertexShaderIcon = lustra::AssetManager::Get().Load<lustra::TextureAsset>("icons/vertexShader.png", true);
    const auto fragmentShaderIcon = lustra::AssetManager::Get().Load<lustra::TextureAsset>("icons/fragmentShader.png", true);

    icons = {
        { "file", fileIcon },
        { "folder", folderIcon },
        { "texture", textureIcon },
        { "material", materialIcon },
        { "model", modelIcon },
        { "script", scriptIcon },
        { "scene", sceneIcon },

        { "play", playIcon },
        { "pause", pauseIcon },
        { "stop", stopIcon },
        { "build", buildIcon },

        { "light", lightIcon },
        { "sound", soundIcon },
        { "camera", cameraIcon },

        { "vertexShader", vertexShaderIcon },
        { "fragmentShader", fragmentShaderIcon }
    };

    assetIcons =
    {
        { lustra::Asset::Type::Texture, textureIcon },
        { lustra::Asset::Type::Material, materialIcon },
        { lustra::Asset::Type::Model, modelIcon },
        { lustra::Asset::Type::Script, scriptIcon },
        { lustra::Asset::Type::Scene, sceneIcon },
        { lustra::Asset::Type::Sound, soundIcon },
        { lustra::Asset::Type::VertexShader, vertexShaderIcon },
        { lustra::Asset::Type::FragmentShader, fragmentShaderIcon },
        { lustra::Asset::Type::Unknown, fileIcon }
    };

    lustra::AssetManager::Get().SetAssetsDirectory(currentDir);
}

void Editor::CreateDefaultEntities()
{
    CreateCameraEntity();
    CreateEditorCameraEntity();
    CreatePostProcessingEntity();
    CreateSkyEntity();
}

void Editor::CreateCameraEntity() const
{
    auto camera = scene->CreateEntity();

    camera.AddComponent<lustra::NameComponent>().name = "Camera";
    camera.AddComponent<lustra::TransformComponent>().position = { 5.0f, 5.0f, 5.0f };

    auto& cameraComponent = camera.AddComponent<lustra::CameraComponent>();

    cameraComponent.camera.SetViewport(window->GetContentSize());
    cameraComponent.camera.SetPerspective();
}

void Editor::CreateEditorCameraEntity()
{
    editorCamera = scene->CreateEntity();

    editorCamera.AddComponent<lustra::NameComponent>().name = "EditorCamera";
    editorCamera.AddComponent<lustra::TransformComponent>().position = { 0.0f, 0.0f, 5.0f };

    auto& cameraComponent = editorCamera.AddComponent<lustra::CameraComponent>();

    cameraComponent.camera.SetViewport(window->GetContentSize());
    cameraComponent.camera.SetPerspective();
    cameraComponent.active = true;

    UpdateEditorCameraScript();
}

void Editor::CreatePostProcessingEntity() const
{
    auto postProcessing = scene->CreateEntity();

    postProcessing.AddComponent<lustra::NameComponent>().name = "PostProcessing";
    postProcessing.AddComponent<lustra::TonemapComponent>(LLGL::Extent2D{ 1280, 720 });
    postProcessing.AddComponent<lustra::BloomComponent>(LLGL::Extent2D{ 1280, 720 });
    postProcessing.AddComponent<lustra::GTAOComponent>(LLGL::Extent2D{ 1280, 720 });
    postProcessing.AddComponent<lustra::SSRComponent>(LLGL::Extent2D{ 1280, 720 });
}

void Editor::CreateSkyEntity() const
{
    auto sky = scene->CreateEntity();

    sky.AddComponent<lustra::NameComponent>().name = "Sky";
    sky.AddComponent<lustra::MeshComponent>().model = lustra::AssetManager::Get().Load<lustra::ModelAsset>("cube", true);

    sky.AddComponent<lustra::ProceduralSkyComponent>(LLGL::Extent2D{ 1024, 1024 });
    /* sky.AddComponent<lustra::HDRISkyComponent>(
        lustra::AssetManager::Get().Load<lustra::TextureAsset>("hdri/meadow_2_1k.hdr", true),
        LLGL::Extent2D{ 1024, 1024 }
    ); */
}

void Editor::UpdateList()
{
    list.clear();

    for(auto entity: scene->GetRegistry().view<entt::entity>())
        list.emplace_back(entity, scene.get());

    editorCamera = scene->GetEntity("EditorCamera");
}

void Editor::UpdateEditorCameraScript()
{
    auto& cameraScript = editorCamera.GetOrAddComponent<lustra::ScriptComponent>();

    // Just a reminder that this is possible ;)
    /* auto& script = camera.AddComponent<lustra::ScriptComponent>();
    script.script = lustra::AssetManager::Get().Load<lustra::ScriptAsset>("camera.as", true);

    lustra::ScriptManager::Get().AddScript(script.script);
    lustra::ScriptManager::Get().Build(); */

    cameraScript.update = [&](lustra::Entity entity, const float deltaTime)
    {
        auto& transform = entity.GetComponent<lustra::TransformComponent>();

        static auto speed = 0.0f;
        static auto multiplier = 1.0f;

        static auto movement = glm::vec3(0.0f);

        if(lustra::Mouse::IsButtonPressed(lustra::Mouse::Button::Right) && canMoveCamera)
        {
            lustra::Mouse::SetCursorVisible(false);

            const auto rotation = glm::quat(glm::radians(transform.rotation));

            auto input = glm::vec3(0.0f);

            if(lustra::Keyboard::IsKeyPressed(lustra::Keyboard::Key::W))
                input -= rotation * glm::vec3(0.0f, 0.0f, 1.0f);
            if(lustra::Keyboard::IsKeyPressed(lustra::Keyboard::Key::S))
                input += rotation * glm::vec3(0.0f, 0.0f, 1.0f);
            if(lustra::Keyboard::IsKeyPressed(lustra::Keyboard::Key::A))
                input -= rotation * glm::vec3(1.0f, 0.0f, 0.0f);
            if(lustra::Keyboard::IsKeyPressed(lustra::Keyboard::Key::D))
                input += rotation * glm::vec3(1.0f, 0.0f, 0.0f);

            if(const auto scroll = lustra::Mouse::Scroll::offset.y; scroll != 0.0f)
                multiplier = glm::clamp(multiplier * (scroll > 0.0 ? 1.2f : 0.9f), 0.1f, 60.0f);

            const float lerpSpeed = glm::clamp(deltaTime * 5.0f, 0.0f, 1.0f);

            if(glm::length(input) > 0.1f)
            {
                speed = glm::mix(speed, 3.0f * multiplier, lerpSpeed);
                movement = input;
            }
            else
                speed = glm::mix(speed, 0.0f, lerpSpeed);

            const auto deltaPosition = glm::normalize(movement) * deltaTime * speed;

            if(speed > 0.0f)
                transform.position += deltaPosition;

            lustra::Listener::SetVelocity(deltaPosition / deltaTime);

            const glm::vec2 center(
                static_cast<float>(window->GetContentSize().width) / 2.0f,
                static_cast<float>(window->GetContentSize().height) / 2.0f
            );
            const glm::vec2 delta = center - lustra::Mouse::GetPosition();

            transform.rotation.x += delta.y / 100.0f;
            transform.rotation.y += delta.x / 100.0f;

            transform.rotation.x = glm::clamp(transform.rotation.x, -89.0f, 89.0f);

            lustra::Mouse::SetPosition(center);
        }
        else
        {
            lustra::Listener::SetVelocity(glm::vec3(0.0f));
            lustra::Mouse::SetCursorVisible();
        }
    };
}

void Editor::SwitchScene(const lustra::SceneAssetPtr& newScene)
{
    this->scene = newScene->scene;

    sceneAsset = newScene;

    if(newScene->scene->GetRegistry().storage().begin() == newScene->scene->GetRegistry().storage().end())
        CreateDefaultEntities();

    UpdateList();

    if(editorCamera)
        UpdateEditorCameraScript();
    else
        CreateEditorCameraEntity();

    lustra::EventManager::Get().Dispatch(
        std::make_unique<lustra::WindowResizeEvent>(lustra::Renderer::Get().GetViewportResolution())
    );
}

void Editor::CreateModelEntity(const lustra::ModelAssetPtr& model, const bool relativeToCamera)
{
    auto entity = scene->CreateEntity();

    entity.AddComponent<lustra::NameComponent>().name = "Model";
    entity.AddComponent<lustra::TransformComponent>();
    entity.AddComponent<lustra::MeshComponent>().model = model;
    entity.AddComponent<lustra::MeshRendererComponent>();
    entity.AddComponent<lustra::PipelineComponent>(
        lustra::AssetManager::Get().Load<lustra::VertexShaderAsset>("vertex.vert", true),
        lustra::AssetManager::Get().Load<lustra::FragmentShaderAsset>("deferred.frag", true)
    );

    auto& rigidBody = entity.AddComponent<lustra::RigidBodyComponent>();

    auto& modelPos = entity.GetComponent<lustra::TransformComponent>().position;

    if(relativeToCamera)
    {
        const auto& cameraPos = editorCamera.GetComponent<lustra::TransformComponent>().position;

        const auto cameraOrient = glm::quat(glm::radians(editorCamera.GetComponent<lustra::TransformComponent>().rotation));

        modelPos = cameraPos + cameraOrient * glm::vec3(0.0f, 0.0f, -5.0f);
    }

    auto settings =
        JPH::BodyCreationSettings(
            new JPH::EmptyShapeSettings(),
            { modelPos.x, modelPos.y, modelPos.z },
            { 0.0f, 0.0f, 0.0f, 1.0f },
            JPH::EMotionType::Dynamic,
            lustra::Layers::moving
        );

    settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
    settings.mMassPropertiesOverride.mMass = 1.0f;

    rigidBody.body = lustra::PhysicsManager::Get().CreateBody(settings);

    selectedEntity = entity;

    list.push_back(entity);
}

void Editor::CreateRenderTarget(const LLGL::Extent2D& resolution)
{
    if(viewportRenderTarget)
    {
        lustra::Renderer::Get().Release(viewportAttachment);
        lustra::Renderer::Get().Release(viewportRenderTarget);
    }

    const LLGL::TextureDescriptor colorAttachmentDesc =
    {
        .type = LLGL::TextureType::Texture2D,
        .bindFlags = LLGL::BindFlags::ColorAttachment,
        .format = LLGL::Format::RGBA8UNorm,
        .extent = { resolution.width, resolution.height, 1 },
        .mipLevels = 1,
        .samples = 1
    };

    viewportAttachment = lustra::Renderer::Get().CreateTexture(colorAttachmentDesc);
    viewportRenderTarget = lustra::Renderer::Get().CreateRenderTarget(resolution, { viewportAttachment });

    LLGL::OpenGL::ResourceNativeHandle nativeHandle{};
    viewportAttachment->GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
    nativeViewportAttachment = nativeHandle.id;
}

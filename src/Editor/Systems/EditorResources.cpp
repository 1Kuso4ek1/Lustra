#include <Editor.hpp>

void Editor::SetupAssetManager()
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

void Editor::LoadIcons()
{
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

void Editor::CreateDefaultEntities()
{
    CreateCameraEntity();
    CreateEditorCameraEntity();
    CreatePostProcessingEntity();
    CreateSkyEntity();
}

void Editor::CreateCameraEntity()
{
    auto camera = scene->CreateEntity();

    camera.AddComponent<dev::NameComponent>().name = "Camera";
    camera.AddComponent<dev::TransformComponent>().position = { 5.0f, 5.0f, 5.0f };
    
    auto& cameraComponent = camera.AddComponent<dev::CameraComponent>();
    
    cameraComponent.camera.SetViewport(window->GetContentSize());
    cameraComponent.camera.SetPerspective();
}

void Editor::CreateEditorCameraEntity()
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

void Editor::CreatePostProcessingEntity()
{
    auto postProcessing = scene->CreateEntity();

    postProcessing.AddComponent<dev::NameComponent>().name = "PostProcessing";
    postProcessing.AddComponent<dev::TonemapComponent>(LLGL::Extent2D{ 1280, 720 });
    postProcessing.AddComponent<dev::BloomComponent>(LLGL::Extent2D{ 1280, 720 });
    postProcessing.AddComponent<dev::GTAOComponent>(LLGL::Extent2D{ 1280, 720 });
    postProcessing.AddComponent<dev::SSRComponent>(LLGL::Extent2D{ 1280, 720 });
}

void Editor::CreateSkyEntity()
{
    auto sky = scene->CreateEntity();
    
    sky.AddComponent<dev::NameComponent>().name = "Sky";
    sky.AddComponent<dev::MeshComponent>().model = dev::AssetManager::Get().Load<dev::ModelAsset>("cube", true);

    sky.AddComponent<dev::ProceduralSkyComponent>(LLGL::Extent2D{ 1024, 1024 });
    /* sky.AddComponent<dev::HDRISkyComponent>(
        dev::AssetManager::Get().Load<dev::TextureAsset>("hdri/meadow_2_1k.hdr", true),
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
    auto& cameraScript = editorCamera.GetOrAddComponent<dev::ScriptComponent>();

    // Just a reminder that this is possible ;)
    /* auto& script = camera.AddComponent<dev::ScriptComponent>();
    script.script = dev::AssetManager::Get().Load<dev::ScriptAsset>("camera.as", true);

    dev::ScriptManager::Get().AddScript(script.script);
    dev::ScriptManager::Get().Build(); */

    cameraScript.update = [&](dev::Entity entity, float deltaTime)
    {
        auto& transform = entity.GetComponent<dev::TransformComponent>();

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

void Editor::CreateModelEntity(dev::ModelAssetPtr model, bool relativeToCamera)
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

void Editor::CreateRenderTarget(const LLGL::Extent2D& resolution)
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

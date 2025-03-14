#include <Editor.hpp>

Editor::Editor(const lustra::Config& config) : lustra::Application(config)
{
    Init();
}

void Editor::Init()
{
    lustra::PhysicsManager::Get().Init();
    
    SetupAssetManager();
    lustra::AssetManager::Get().LaunchWatch();

    lustra::EventManager::Get().AddListener(lustra::Event::Type::WindowResize, this);
    lustra::EventManager::Get().AddListener(lustra::Event::Type::WindowFocus, this);
    lustra::EventManager::Get().AddListener(lustra::Event::Type::AssetLoaded, this);

    CreateRenderTarget();

    LoadIcons();

    deferredRenderer = std::make_shared<lustra::DeferredRenderer>();

    auto mainScenePath = 
        lustra::AssetManager::Get().GetAssetPath<lustra::SceneAsset>(config.mainScene, true);

    if(!std::filesystem::exists(mainScenePath))
    {
        scene = std::make_shared<lustra::Scene>(deferredRenderer);
        sceneAsset = std::make_shared<lustra::SceneAsset>(scene);
        sceneAsset->path = mainScenePath;

        CreateDefaultEntities();

        UpdateList();

        lustra::AssetManager::Get().Write(sceneAsset);
    }
    else
        SwitchScene(lustra::AssetManager::Get().Load<lustra::SceneAsset>(mainScenePath));
}

void Editor::Update(float deltaTime)
{
    using namespace lustra;

    static auto checkShortcut =
        [](const std::initializer_list<Keyboard::Key> shortcut)
        {
            static Timer timer;

            if(timer.GetElapsedSeconds() < 0.2f)
                return false;

            for(auto key: shortcut)
                if(!Keyboard::IsKeyPressed(key))
                    return false;

            timer.Reset();

            return true;
        };

    if(playing && !paused)
    {
        scene->GetRegistry().view<CameraComponent>().each(
            [](auto entity, auto& component)
            {
                component.active = true;
            }
        );

        editorCamera.GetComponent<CameraComponent>().active = false;

        scene->Update(deltaTime);
    }
    else
    {
        editorCamera.GetComponent<CameraComponent>().active = true;
        editorCamera.GetComponent<ScriptComponent>().update(editorCamera, deltaTime);
    }

    if(checkShortcut({ Keyboard::Key::G }))
    {
        scene->ToggleUpdatePhysics();
        keyboardTimer.Reset();
    }

    if((checkShortcut({ Keyboard::Key::LeftControl, Keyboard::Key::S }) ||
        sceneSaveTimer.GetElapsedSeconds() >= 10.0f) && !playing)
    {
        AssetManager::Get().Write(sceneAsset);
        keyboardTimer.Reset();
        sceneSaveTimer.Reset();
    }
}

void Editor::Render()
{
    scene->Draw(viewportRenderTarget);

    lustra::Renderer::Get().ClearRenderTarget();

    DrawImGui();

    lustra::Renderer::Get().Present();
}

void Editor::OnEvent(lustra::Event& event)
{
    if(event.GetType() == lustra::Event::Type::WindowResize)
    {
        auto resizeEvent = dynamic_cast<lustra::WindowResizeEvent*>(&event);

        CreateRenderTarget(resizeEvent->GetSize());
    }
    else if(event.GetType() == lustra::Event::Type::WindowFocus)
    {
        auto focusEvent = dynamic_cast<lustra::WindowFocusEvent*>(&event);

        if(focusEvent->IsFocused())
            lustra::Renderer::Get().GetSwapChain()->SetVsyncInterval(config.vsync ? 1 : 0);
        else
            lustra::Renderer::Get().GetSwapChain()->SetVsyncInterval(5);
    }
    else if(event.GetType() == lustra::Event::Type::AssetLoaded)
    {
        auto assetLoadedEvent = dynamic_cast<lustra::AssetLoadedEvent*>(&event);

        if(assetLoadedEvent->GetAsset() == sceneAsset)
            SwitchScene(sceneAsset);
    }
}

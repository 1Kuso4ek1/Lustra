#include <Editor.hpp>

Editor::Editor(const lustra::Config& config) : lustra::Application(config)
{
    Init();
}

void Editor::Init()
{
    if(!window->IsFullscreen())
        window->Maximize();

    lustra::PhysicsManager::Get().Init();
    
    SetupAssetManager();
    lustra::AssetManager::Get().LaunchWatch();

    lustra::EventManager::Get().AddListener(lustra::Event::Type::WindowResize, this);
    lustra::EventManager::Get().AddListener(lustra::Event::Type::WindowFocus, this);
    lustra::EventManager::Get().AddListener(lustra::Event::Type::AssetLoaded, this);

    CreateRenderTarget();

    LoadIcons();

    auto mainScenePath = 
        lustra::AssetManager::Get().GetAssetPath<lustra::SceneAsset>(config.mainScene, true);

    if(!std::filesystem::exists(mainScenePath))
    {
        scene = std::make_shared<lustra::Scene>();
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

    if(checkShortcut({ Keyboard::Key::F11 }))
    {
        fullscreenViewport = !fullscreenViewport;

        canMoveCamera = true;

        lustra::WindowResizeEvent event(
            fullscreenViewport ? window->GetContentSize() : lustra::Renderer::Get().GetViewportResolution()
        );

        lustra::EventManager::Get().Dispatch(std::make_unique<lustra::WindowResizeEvent>(event));
    }

    if(checkShortcut({ Keyboard::Key::F5 }))
    {
        playing = !playing;
        paused = false;

        if(playing)
        {
            scene->SetIsRunning(true);
            scene->Start();
        }
        else
            scene->SetIsRunning(false);

        keyboardTimer.Reset();
    }

    if(checkShortcut({ Keyboard::Key::F6 }) && playing)
    {
        paused = !paused;
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
    scene->Draw(fullscreenViewport ? lustra::Renderer::Get().GetSwapChain() : viewportRenderTarget);
}

void Editor::OnEvent(lustra::Event& event)
{
    if(event.GetType() == lustra::Event::Type::WindowResize)
    {
        auto resizeEvent = dynamic_cast<lustra::WindowResizeEvent*>(&event);

        lustra::Renderer::Get().SetViewportResolution(resizeEvent->GetSize());

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

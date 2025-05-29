#include <Editor.hpp>

Editor::Editor(const lustra::Config& config) : lustra::Application(config)
{
    Init();
}

void Editor::Init()
{
    if(!window->IsFullscreen())
        window->Maximize();

    SetupAssetManager();

    lustra::AssetManager::Get().LaunchWatch();

    lustra::EventManager::Get().AddListener(lustra::Event::Type::WindowResize, this);
    lustra::EventManager::Get().AddListener(lustra::Event::Type::WindowFocus, this);
    lustra::EventManager::Get().AddListener(lustra::Event::Type::AssetLoaded, this);

    CreateRenderTarget();

    LoadIcons();

    const auto mainScenePath =
        lustra::AssetManager::Get().GetAssetPath<lustra::SceneAsset>(config.mainScene, true);

    lustra::PhysicsManager::Get().Init();

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

void Editor::Update(const float deltaTime)
{
    using namespace lustra;

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
    else// if(editorCamera.HasComponent<CameraComponent>())
    {
        editorCamera.GetComponent<CameraComponent>().active = true;
        editorCamera.GetComponent<ScriptComponent>().update(editorCamera, deltaTime);
    }

    if(CheckShortcut({ Keyboard::Key::G }))
    {
        scene->ToggleUpdatePhysics();
        keyboardTimer.Reset();
    }

    if(CheckShortcut({ Keyboard::Key::F11 }))
    {
        fullscreenViewport = !fullscreenViewport;

        canMoveCamera = true;

        lustra::WindowResizeEvent event(
            fullscreenViewport ? window->GetContentSize() : lustra::Renderer::Get().GetViewportResolution()
        );

        lustra::EventManager::Get().Dispatch(std::make_unique<lustra::WindowResizeEvent>(event));
    }

    if((CheckShortcut({ Keyboard::Key::LeftControl, Keyboard::Key::S }) ||
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
        const auto resizeEvent = dynamic_cast<lustra::WindowResizeEvent*>(&event);

        lustra::Renderer::Get().SetViewportResolution(resizeEvent->GetSize());

        CreateRenderTarget(resizeEvent->GetSize());
    }
    else if(event.GetType() == lustra::Event::Type::WindowFocus)
    {
        const auto focusEvent = dynamic_cast<lustra::WindowFocusEvent*>(&event);

        if(focusEvent->IsFocused())
            lustra::Renderer::Get().GetSwapChain()->SetVsyncInterval(config.vsync ? 1 : 0);
        else
            lustra::Renderer::Get().GetSwapChain()->SetVsyncInterval(5);
    }
    else if(event.GetType() == lustra::Event::Type::AssetLoaded)
    {
        const auto assetLoadedEvent = dynamic_cast<lustra::AssetLoadedEvent*>(&event);

        if(assetLoadedEvent->GetAsset() == sceneAsset)
            SwitchScene(sceneAsset);
    }
}

bool Editor::CheckShortcut(const std::initializer_list<lustra::Keyboard::Key> shortcut)
{
    static lustra::Timer timer;

    if(timer.GetElapsedSeconds() < 0.2f)
        return false;

    for(const auto key : shortcut)
        if(!lustra::Keyboard::IsKeyPressed(key))
            return false;

    timer.Reset();

    return true;
}

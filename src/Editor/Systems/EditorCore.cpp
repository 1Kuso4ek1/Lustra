#include <Editor.hpp>

Editor::Editor(const lustra::Config& config) : lustra::Application(config)
{
    Init();
}

void Editor::Init()
{
    lustra::EventManager::Get().AddListener(lustra::Event::Type::WindowResize, this);
    lustra::EventManager::Get().AddListener(lustra::Event::Type::WindowFocus, this);

    CreateRenderTarget();

    LoadIcons();

    deferredRenderer = std::make_shared<lustra::DeferredRenderer>();

    auto mainScenePath = lustra::AssetManager::Get().GetAssetPath<lustra::SceneAsset>("main.scn", true);

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
    {
        sceneAsset = lustra::AssetManager::Get().Load<lustra::SceneAsset>(mainScenePath);
        scene = sceneAsset->scene;
        scene->SetRenderer(deferredRenderer);

        UpdateList();

        UpdateEditorCameraScript();
    }
}

void Editor::Update(float deltaTime)
{
    if(playing && !paused)
    {
        scene->GetRegistry().view<lustra::CameraComponent>().each(
            [](auto entity, auto& component)
            {
                component.active = true;
            }
        );

        editorCamera.GetComponent<lustra::CameraComponent>().active = false;

        scene->Update(deltaTime);
    }
    else
    {
        editorCamera.GetComponent<lustra::CameraComponent>().active = true;
        editorCamera.GetComponent<lustra::ScriptComponent>().update(editorCamera, deltaTime);
    }

    if(lustra::Keyboard::IsKeyPressed(lustra::Keyboard::Key::G) &&
       keyboardTimer.GetElapsedSeconds() > 0.2f)
    {
        scene->ToggleUpdatePhysics();
        keyboardTimer.Reset();
    }

    if((lustra::Keyboard::IsKeyPressed(lustra::Keyboard::Key::S) &&
       lustra::Keyboard::IsKeyPressed(lustra::Keyboard::Key::LeftControl) &&
       keyboardTimer.GetElapsedSeconds() > 0.2f) ||
       sceneSaveTimer.GetElapsedSeconds() >= 10.0f)
    {
        lustra::AssetManager::Get().Write(sceneAsset);
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
}

#include <Editor.hpp>

Editor::Editor(const dev::Config& config) : dev::Application(config)
{
    Init();
}

void Editor::Init()
{
    dev::EventManager::Get().AddListener(dev::Event::Type::WindowResize, this);
    dev::EventManager::Get().AddListener(dev::Event::Type::WindowFocus, this);

    CreateRenderTarget();

    LoadIcons();

    deferredRenderer = std::make_shared<dev::DeferredRenderer>();

    auto mainScenePath = dev::AssetManager::Get().GetAssetPath<dev::SceneAsset>("main.scn", true);

    if(!std::filesystem::exists(mainScenePath))
    {
        scene = std::make_shared<dev::Scene>(deferredRenderer);
        sceneAsset = std::make_shared<dev::SceneAsset>(scene);
        sceneAsset->path = mainScenePath;

        CreateDefaultEntities();

        UpdateList();

        dev::AssetManager::Get().Write(sceneAsset);
    }
    else
    {
        sceneAsset = dev::AssetManager::Get().Load<dev::SceneAsset>(mainScenePath);
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
        scene->GetRegistry().view<dev::CameraComponent>().each(
            [](auto entity, auto& component)
            {
                component.active = true;
            }
        );

        editorCamera.GetComponent<dev::CameraComponent>().active = false;

        scene->Update(deltaTime);
    }
    else
    {
        editorCamera.GetComponent<dev::CameraComponent>().active = true;
        editorCamera.GetComponent<dev::ScriptComponent>().update(editorCamera, deltaTime);
    }

    if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::G) &&
       keyboardTimer.GetElapsedSeconds() > 0.2f)
    {
        scene->ToggleUpdatePhysics();
        keyboardTimer.Reset();
    }

    if((dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::S) &&
       dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::LeftControl) &&
       keyboardTimer.GetElapsedSeconds() > 0.2f) ||
       sceneSaveTimer.GetElapsedSeconds() >= 10.0f)
    {
        dev::AssetManager::Get().Write(sceneAsset);
        keyboardTimer.Reset();
        sceneSaveTimer.Reset();
    }
}

void Editor::Render()
{
    scene->Draw(viewportRenderTarget);

    dev::Renderer::Get().ClearRenderTarget();

    DrawImGui();

    dev::Renderer::Get().Present();
}

void Editor::OnEvent(dev::Event& event)
{
    if(event.GetType() == dev::Event::Type::WindowResize)
    {
        auto resizeEvent = dynamic_cast<dev::WindowResizeEvent*>(&event);

        CreateRenderTarget(resizeEvent->GetSize());
    }
    else if(event.GetType() == dev::Event::Type::WindowFocus)
    {
        auto focusEvent = dynamic_cast<dev::WindowFocusEvent*>(&event);

        if(focusEvent->IsFocused())
            dev::Renderer::Get().GetSwapChain()->SetVsyncInterval(config.vsync ? 1 : 0);
        else
            dev::Renderer::Get().GetSwapChain()->SetVsyncInterval(5);
    }
}

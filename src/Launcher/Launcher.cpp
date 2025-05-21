#include <Launcher.hpp>

Launcher::Launcher(const lustra::Config& config) : Application(config)
{
    Launcher::Init();
}

void Launcher::Init()
{
    SetupAssetManager();

    lustra::EventManager::Get().AddListener(lustra::Event::Type::WindowFocus, this);

    lustra::PhysicsManager::Get().Init();

    sceneAsset = lustra::AssetManager::Get().Load<lustra::SceneAsset>(config.mainScene, true);

    scene = sceneAsset->scene;

    lustra::EventManager::Get().Dispatch(
        std::make_unique<lustra::WindowResizeEvent>(lustra::Renderer::Get().GetSwapChain()->GetResolution())
    );

    scene->GetRegistry().view<lustra::CameraComponent>().each(
        [](auto entity, auto& component)
        {
            component.active = true;
        }
    );

    scene->SetIsRunning(true);
    scene->SetUpdatePhysics(true);

    scene->Start();
}

void Launcher::Update(const float deltaTime)
{
    scene->Update(deltaTime);
}

void Launcher::Render()
{
    scene->Draw(lustra::Renderer::Get().GetSwapChain());
}

void Launcher::OnEvent(lustra::Event& event)
{
    if(event.GetType() == lustra::Event::Type::WindowFocus)
    {
        const auto focusEvent = dynamic_cast<lustra::WindowFocusEvent*>(&event);

        if(focusEvent->IsFocused())
            lustra::Renderer::Get().GetSwapChain()->SetVsyncInterval(config.vsync ? 1 : 0);
        else
            lustra::Renderer::Get().GetSwapChain()->SetVsyncInterval(5);
    }
}

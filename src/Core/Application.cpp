#include <Application.hpp>

namespace dev
{

Application::Application(const Config& config) : config(config)
{
    LLGL::Log::RegisterCallbackStd(LLGL::Log::StdOutFlags::Colored);

    ScopedTimer timer("App initialization");

    window = std::make_shared<Window>(config.resolution, config.title, 1, config.fullscreen);

    if(!Renderer::Get().IsInit())
        return;

    Renderer::Get().InitSwapChain(window);

    if(config.vsync)
        Renderer::Get().GetSwapChain()->SetVsyncInterval(1);

    ImGuiManager::Get().Init(
        window->GetGLFWWindow(),
        config.imGuiFontPath,
        config.imGuiLayoutPath
    );

    PhysicsManager::Get().Init();

    SetupAssetManager();
}

Application::~Application()
{
    if(Renderer::Get().IsInit())
    {
        ImGuiManager::Get().Destroy();

        Renderer::Get().Unload();
    }

    config.Save(config.configPath);
}

void Application::Run()
{
    if(!Renderer::Get().IsInit())
    {
        LLGL::Log::Errorf(LLGL::Log::ColorFlags::StdError, "Error: Renderer is not initialized\n");
        return;
    }

    while(window->PollEvents())
    {
        LLGL::Surface::ProcessEvents();
        
        Multithreading::Get().Update();

        Update(deltaTimeTimer.GetElapsedSeconds());

        deltaTimeTimer.Reset();

        Render();
    }
}

void Application::SetupAssetManager()
{
    AssetManager::Get().SetAssetsDirectory(config.assetsRoot);

    AssetManager::Get().AddLoader<TextureAsset, TextureLoader>("textures");
    AssetManager::Get().AddLoader<MaterialAsset, MaterialLoader>("materials");
    AssetManager::Get().AddLoader<ModelAsset, ModelLoader>("models");
    AssetManager::Get().AddLoader<ScriptAsset, ScriptLoader>("scripts");
    AssetManager::Get().AddLoader<VertexShaderAsset, VertexShaderLoader>("shaders");
    AssetManager::Get().AddLoader<FragmentShaderAsset, FragmentShaderLoader>("shaders");
    AssetManager::Get().AddLoader<SceneAsset, SceneLoader>("scenes");
}

}

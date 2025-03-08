#pragma once
#include <Config.hpp>

#include <Timer.hpp>
#include <Window.hpp>

#include <Renderer.hpp>
#include <ImGuiManager.hpp>
#include <PhysicsManager.hpp>
#include <AssetManager.hpp>

#include <TextureLoader.hpp>
#include <MaterialLoader.hpp>
#include <ModelLoader.hpp>
#include <ScriptLoader.hpp>
#include <ShaderLoader.hpp>
#include <SceneLoader.hpp>
#include <SoundLoader.hpp>

namespace lustra
{

class Application
{
public:
    Application(const Config& config = {});
    virtual ~Application();

    virtual void Run();

    virtual void Init() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;

protected:
    void SetupAssetManager();

protected:
    Config config;

    WindowPtr window;

    Timer deltaTimeTimer;
};

}

#pragma once
#include <Application.hpp>

#include <Scene.hpp>
#include <Entity.hpp>

#include <Window.hpp>

#include <Keyboard.hpp>
#include <Mouse.hpp>

#include <ImGuizmo.h>

#include <ImGuiManager.hpp>
#include <ComponentsUI.hpp>

#include <TextureLoader.hpp>
#include <ModelLoader.hpp>
#include <AssetManager.hpp>

class SceneTestApp : public dev::Application
{
public:
    SceneTestApp();
    ~SceneTestApp();

    void Run() override;

private:
    void LoadShaders();
    void LoadTextures();

    void CreateEntities();

    void CreateRifleEntity();
    void CreateCameraEntity();
    void CreatePostProcessingEntity();
    void CreateLightEntity();
    void CreateLight1Entity();
    void CreateSkyEntity();
    
    void DrawImGui();

    void DrawSceneTree();
    void DrawImGuizmoControls();
    void DrawImGuizmo();
    void DrawTextureViewer();

    void Draw();

    ImGuizmo::OPERATION currentOperation = ImGuizmo::OPERATION::TRANSLATE;
    float snap[3] = { 1.0f, 1.0f, 1.0f };

    dev::Entity rifle, camera, postProcessing, light, light1, sky, selectedEntity;

    std::vector<dev::Entity> list;

    dev::Scene scene;

    dev::WindowPtr window;

    dev::Timer deltaTimeTimer, keyboardTimer;

    LLGL::PipelineState* pipeline{};

    LLGL::Shader* vertexShader{};
    LLGL::Shader* fragmentShader{};

    std::shared_ptr<dev::TextureAsset> texture, metal, wood;
};

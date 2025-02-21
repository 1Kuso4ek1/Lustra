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

#include <ScriptManager.hpp>

#include <TextureLoader.hpp>
#include <MaterialLoader.hpp>
#include <ModelLoader.hpp>
#include <ScriptLoader.hpp>
#include <ShaderLoader.hpp>
#include <SceneLoader.hpp>
#include <AssetManager.hpp>

#include <LLGL/Backend/OpenGL/NativeHandle.h>

class SceneTestApp : public dev::Application, public dev::EventListener
{
public:
    SceneTestApp();
    ~SceneTestApp();

    void Run() override;

private:
    void SetupAssetManager();

    void LoadTextures();

    void CreateEntities();

    void CreateRifleEntity();
    void CreateCameraEntity();
    void CreateEditorCameraEntity();
    void CreatePostProcessingEntity();
    void CreateLightEntity();
    void CreateLight1Entity();
    void CreateSkyEntity();

    void UpdateList();
    void UpdateEditorCameraScript();

    void CreateModelEntity(dev::ModelAssetPtr model, bool relativeToCamera = false);

    void CreateRenderTarget(const LLGL::Extent2D& resolution = dev::Renderer::Get().GetViewportResolution());

    void DrawImGui();

    void DrawSceneTree();
    void DrawEntityNode(dev::Entity entity);
    void EntityNodeInteraction(dev::Entity entity, std::string_view name);

    void DrawPropertiesWindow();
    void DrawImGuizmoControls();
    void DrawImGuizmo();
    void DrawExecutionControl();

    void DrawMaterialPreview(dev::MaterialAssetPtr material, const ImVec2& size);

    void DrawAssetBrowser();
    void DrawAsset(const std::filesystem::path& entry, dev::AssetPtr asset);
    void DrawUnloadedAsset(const std::filesystem::path& entry);

    void DrawCreateAssetMenu(const std::filesystem::path& currentDirectory);
    bool DrawCreateMaterialMenu(const std::filesystem::path& currentDirectory);
    bool DrawCreateScriptMenu(const std::filesystem::path& currentDirectory);
    bool DrawCreateSceneMenu(const std::filesystem::path& currentDirectory);

    void DrawMaterialEditor(dev::MaterialAssetPtr material);
    void DrawMaterialProperty(dev::MaterialAsset::Property& property, int id, bool singleComponent = false);
    
    void DrawViewport();

    void Draw();

    void OnEvent(dev::Event& event) override;

private:
    bool playing = false, paused = false;

private:
    ImGuizmo::OPERATION currentOperation = ImGuizmo::OPERATION::TRANSLATE;
    float snap[3] = { 1.0f, 1.0f, 1.0f };

private:
    bool canMoveCamera = false;

private:
    dev::Entity editorCamera, selectedEntity;

    std::vector<dev::Entity> list;

private:
    std::shared_ptr<dev::DeferredRenderer> deferredRenderer;

    dev::SceneAssetPtr sceneAsset;
    std::shared_ptr<dev::Scene> scene;

    dev::WindowPtr window;

    dev::Timer deltaTimeTimer, keyboardTimer;

private:
    GLuint nativeViewportAttachment;

    LLGL::Texture* viewportAttachment{};
    LLGL::RenderTarget* viewportRenderTarget{};

private:
    dev::AssetPtr selectedAsset;

    dev::TextureAssetPtr texture, metal, wood,
                         fileIcon, folderIcon, textureIcon,
                         materialIcon, modelIcon, scriptIcon,
                         playIcon, pauseIcon, stopIcon,
                         buildIcon, lightIcon, sceneIcon;
    dev::MaterialAssetPtr ak47Metal, ak47Wood;

    std::unordered_map<dev::Asset::Type, dev::TextureAssetPtr> assetIcons;
};

#pragma once
#include <Application.hpp>
#include <Scene.hpp>
#include <Entity.hpp>

#include <Keyboard.hpp>
#include <Mouse.hpp>

#include <ImGuizmo.h>

#include <ComponentsUI.hpp>

#include <ScriptManager.hpp>

#include <LLGL/Backend/OpenGL/NativeHandle.h>

class Editor : public lustra::Application, public lustra::EventListener
{
public:
    Editor(const lustra::Config& config);

    void Init() override;
    void Update(float deltaTime) override;
    void Render() override;
    void RenderImGui() override;

private:
    bool CheckShortcut(const std::initializer_list<lustra::Keyboard::Key> shortcut);

    void LoadIcons();

    void CreateDefaultEntities();

    void CreateCameraEntity();
    void CreateEditorCameraEntity();
    void CreatePostProcessingEntity();
    void CreateSkyEntity();

    void UpdateList();
    void UpdateEditorCameraScript();

    void SwitchScene(lustra::SceneAssetPtr scene);

    void CreateModelEntity(lustra::ModelAssetPtr model, bool relativeToCamera = false);

    void CreateRenderTarget(const LLGL::Extent2D& resolution = lustra::Renderer::Get().GetViewportResolution());

    void DrawSceneTree();
    void DrawEntityNode(lustra::Entity entity);
    void EntityNodeInteraction(const lustra::Entity& entity, std::string_view name);

    void DrawPropertiesWindow();
    void DrawImGuizmoControls();
    void DrawImGuizmo();
    void DrawExecutionControl();

    void DrawLog();

    void DrawMaterialPreview(lustra::MaterialAssetPtr material, const ImVec2& size);

    void DrawAssetBrowser();
    void DrawAsset(const std::filesystem::path& entry, lustra::AssetPtr asset);
    void DrawUnloadedAsset(const std::filesystem::path& entry);

    void DrawCreateAssetMenu(const std::filesystem::path& currentDirectory);
    bool DrawCreateMaterialMenu(const std::filesystem::path& currentDirectory);
    bool DrawCreateScriptMenu(const std::filesystem::path& currentDirectory);
    bool DrawCreateSceneMenu(const std::filesystem::path& currentDirectory);

    void DrawMaterialEditor(lustra::MaterialAssetPtr material);
    void DrawMaterialProperty(lustra::MaterialAsset::Property& property, int id, bool singleComponent = false);
    
    void DrawViewport();
    void DrawOnScreenIcons();

    void OnEvent(lustra::Event& event) override;

private:
    lustra::Entity buffer;

private:
    bool playing = false, paused = false;

private:
    bool fullscreenViewport = false;

private:
    ImGuizmo::OPERATION currentOperation = ImGuizmo::OPERATION::TRANSLATE;
    float snap[3] = { 1.0f, 1.0f, 1.0f };

private:
    bool canMoveCamera = false;

private:
    lustra::Entity editorCamera, selectedEntity;

    std::vector<lustra::Entity> list;

private:
    lustra::SceneAssetPtr sceneAsset;
    std::shared_ptr<lustra::Scene> scene;

    lustra::Timer keyboardTimer, sceneSaveTimer;

private:
    GLuint nativeViewportAttachment;

    LLGL::Texture* viewportAttachment{};
    LLGL::RenderTarget* viewportRenderTarget{};

private:
    lustra::AssetPtr selectedAsset;

    lustra::TextureAssetPtr texture, metal, wood,
                            fileIcon, folderIcon, textureIcon,
                            materialIcon, modelIcon, scriptIcon,
                            playIcon, pauseIcon, stopIcon,
                            buildIcon, lightIcon, sceneIcon,
                            soundIcon, vertexShaderIcon, fragmentShaderIcon,
                            cameraIcon;

    std::unordered_map<lustra::Asset::Type, lustra::TextureAssetPtr> assetIcons;
};

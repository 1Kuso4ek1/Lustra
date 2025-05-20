#pragma once
#include <Application.hpp>
#include <ComponentsUI.hpp>
#include <Entity.hpp>
#include <ImGuizmo.h>
#include <Keyboard.hpp>
#include <Scene.hpp>

#include <LLGL/Backend/OpenGL/NativeHandle.h>

class Editor final : public lustra::Application, public lustra::EventListener
{
public:
    explicit Editor(const lustra::Config& config);

    void Init() override;
    void Update(float deltaTime) override;
    void Render() override;
    void RenderImGui() override;

private:
    bool CheckShortcut(const std::initializer_list<lustra::Keyboard::Key> shortcut);

    void LoadIcons();

    void CreateDefaultEntities();

    void CreateCameraEntity() const;
    void CreateEditorCameraEntity();
    void CreatePostProcessingEntity() const;
    void CreateSkyEntity() const;

    void UpdateList();
    void UpdateEditorCameraScript();

    void SwitchScene(const lustra::SceneAssetPtr& scene);

    void CreateModelEntity(const lustra::ModelAssetPtr& model, bool relativeToCamera = false);

    void CreateRenderTarget(const LLGL::Extent2D& resolution = lustra::Renderer::Get().GetViewportResolution());

    void DrawSceneTree();
    void DrawEntityNode(lustra::Entity entity);
    void EntityNodeInteraction(const lustra::Entity& entity, std::string_view name);

    void DrawPropertiesWindow();
    void DrawImGuizmoControls();
    void DrawImGuizmo();
    void DrawExecutionControl();

    void DrawLog();

    void DrawMaterialPreview(const lustra::MaterialAssetPtr& material, const ImVec2& size);

    void DrawAssetBrowser();
    void DrawAsset(const std::filesystem::path& entry, const lustra::AssetPtr& asset);
    void DrawUnloadedAsset(const std::filesystem::path& entry);

    static void DrawCreateAssetMenu(const std::filesystem::path& currentDirectory);
    static bool DrawCreateMaterialMenu(const std::filesystem::path& currentDirectory);
    static bool DrawCreateScriptMenu(const std::filesystem::path& currentDirectory);
    static bool DrawCreateSceneMenu(const std::filesystem::path& currentDirectory);
    static void DrawMaterialEditor(const lustra::MaterialAssetPtr& material);
    static void DrawMaterialProperty(lustra::MaterialAsset::Property& property, int id, bool singleComponent = false);

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

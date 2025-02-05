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
#include <MaterialLoader.hpp>
#include <ModelLoader.hpp>
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

    void LoadShaders();
    void LoadTextures();

    void CreateEntities();

    void CreateRifleEntity();
    void CreateCameraEntity();
    void CreatePostProcessingEntity();
    void CreateLightEntity();
    void CreateLight1Entity();
    void CreateSkyEntity();

    void CreateModelEntity(dev::ModelAssetPtr model, bool relativeToCamera = false);

    void CreateRenderTarget(const LLGL::Extent2D& resolution = dev::Renderer::Get().GetSwapChain()->GetResolution());

    void DrawImGui();

    void DrawSceneTree();
    void DrawPropertiesWindow();
    void DrawImGuizmoControls();
    void DrawImGuizmo();

    void DrawMaterialPreview(dev::MaterialAssetPtr material, const ImVec2& size);

    void DrawAssetBrowser();
    void DrawAsset(const std::filesystem::path& entry, dev::AssetPtr asset);
    void DrawUnloadedAsset(const std::filesystem::path& entry);

    void DrawCreateAssetMenu(const std::filesystem::path& currentDirectory);

    void DrawMaterialEditor(dev::MaterialAssetPtr material);
    void DrawMaterialProperty(dev::MaterialAsset::Property& property, int id, bool singleComponent = false);
    
    void DrawViewport();

    void Draw();

    void OnEvent(dev::Event& event) override;

private:
    ImGuizmo::OPERATION currentOperation = ImGuizmo::OPERATION::TRANSLATE;
    float snap[3] = { 1.0f, 1.0f, 1.0f };

    bool canMoveCamera = false;

    dev::Entity rifle, camera, postProcessing, light, light1, sky, selectedEntity;

    std::vector<dev::Entity> list;

    std::shared_ptr<dev::DeferredRenderer> deferredRenderer;

    dev::Scene scene;

    dev::WindowPtr window;

    dev::Timer deltaTimeTimer, keyboardTimer;

    LLGL::PipelineState* pipeline{};

    LLGL::Shader* vertexShader{};
    LLGL::Shader* fragmentShader{};

    GLuint nativeViewportAttachment;

    LLGL::Texture* viewportAttachment{};
    LLGL::RenderTarget* viewportRenderTarget{};

    dev::AssetPtr selectedAsset;

    dev::TextureAssetPtr texture, metal, wood, fileIcon, folderIcon, textureIcon, materialIcon, modelIcon;
    dev::MaterialAssetPtr ak47Metal, ak47Wood;

    std::unordered_map<dev::Asset::Type, dev::TextureAssetPtr> assetIcons;
};

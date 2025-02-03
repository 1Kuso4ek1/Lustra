#pragma once
#include <Mesh.hpp>
#include <ImGuiManager.hpp>
#include <Camera.hpp>
#include <PostProcessing.hpp>
#include <PBRManager.hpp>
#include <AssetManager.hpp>
#include <TextureAsset.hpp>
#include <MaterialAsset.hpp>
#include <ModelAsset.hpp>

#include <entt/entt.hpp>

namespace dev
{

class Entity;

struct ComponentBase
{
    ComponentBase(const std::string_view& componentName) : componentName(componentName) {}
    virtual ~ComponentBase() = default;

    std::string_view componentName;
};

struct NameComponent : public ComponentBase
{
    NameComponent() : ComponentBase("NameComponent") {}

    std::string name;
};

struct TransformComponent : public ComponentBase
{
    TransformComponent() : ComponentBase("TransformComponent") {}

    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
    glm::vec3 scale =    { 1.0f, 1.0f, 1.0f };

    void SetTransform(const glm::mat4& transform);
    
    glm::mat4 GetTransform() const;
};

struct MeshComponent : public ComponentBase
{
    MeshComponent() : ComponentBase("MeshComponent") {}

    ModelAssetPtr model;
};

struct MeshRendererComponent : public ComponentBase
{
    MeshRendererComponent() : ComponentBase("MeshRendererComponent")
    {
        materials.push_back(AssetManager::Get().Load<MaterialAsset>("default", true));
    }

    std::vector<MaterialAssetPtr> materials;
};

struct PipelineComponent : public ComponentBase
{
    PipelineComponent() : ComponentBase("PipelineComponent") {}

    LLGL::PipelineState* pipeline{};
};

struct CameraComponent : public ComponentBase
{
    CameraComponent() : ComponentBase("CameraComponent") {}

    Camera camera;
};

struct LightComponent : public ComponentBase
{
public:
    LightComponent();

    void SetupShadowMap(const LLGL::Extent2D& resolution);

    glm::vec3 color = { 1.0f, 1.0f, 1.0f };

    float intensity = 1.0f;
    float cutoff = 0.0f, outerCutoff = 0.0f;
    float bias = 0.00001f;

    bool shadowMap = false;

    LLGL::Extent2D resolution;

    LLGL::Texture* depth{};
    LLGL::RenderTarget* renderTarget{};
    LLGL::PipelineState* shadowMapPipeline{};

    // Make it a single light space matrix
    glm::mat4 projection;

private:
    void CreateDepth(const LLGL::Extent2D& resolution);
    void CreateRenderTarget(const LLGL::Extent2D& resolution);
    void CreatePipeline();
};

struct ScriptComponent : public ComponentBase
{
    ScriptComponent() : ComponentBase("ScriptComponent") {}

    std::function<void()> start;
    std::function<void(Entity self, float)> update;
};

struct TonemapComponent : public ComponentBase
{
    TonemapComponent(
        const LLGL::Extent2D& resolution = Renderer::Get().GetSwapChain()->GetResolution(),
        bool registerEvent = true
    );

    int algorithm = 0;
    float exposure = 1.0f;

    glm::vec3 colorGrading = { 0.0f, 0.0f, 0.0f };

    float colorGradingIntensity = 0.0f;
    float chromaticAberration = 0.0f;
    float vignetteIntensity = 0.0f;
    float vignetteRoundness = 0.0f;
    float filmGrain = 0.0f;
    float contrast = 1.0f;
    float saturation = 1.0f;
    float brightness = 1.0f;

    PostProcessingPtr postProcessing;

    std::function<void(LLGL::CommandBuffer*)> setUniforms;
};

struct BloomComponent : public ComponentBase, public EventListener
{
    BloomComponent(const LLGL::Extent2D& resolution);
    ~BloomComponent();

    void SetupPostProcessing();
    void OnEvent(Event& event) override;

    float threshold = 1.0f, strength = 0.3f, resolutionScale = 8.0f;

    LLGL::Extent2D resolution;

    LLGL::Sampler* sampler;

    PostProcessingPtr thresholdPass;

    std::array<PostProcessingPtr, 2> pingPong;

    std::function<void(LLGL::CommandBuffer*)> setThresholdUniforms;
};

struct GTAOComponent : public ComponentBase, public EventListener
{
    GTAOComponent(const LLGL::Extent2D& resolution);
    ~GTAOComponent();

    void SetupPostProcessing();
    void OnEvent(Event& event) override;

    float resolutionScale = 2.0f;

    int samples = 4.0f;
    
    float limit = 100.0f;
    float radius = 8.0f;
    float falloff = 1.5f;
    float thicknessMix = 0.2f;
    float maxStride = 8.0f;

    LLGL::Extent2D resolution;

    PostProcessingPtr gtao, boxBlur;
};

struct SSRComponent : public ComponentBase, public EventListener
{
    SSRComponent(const LLGL::Extent2D& resolution);
    ~SSRComponent();

    void SetupPostProcessing();
    void OnEvent(Event& event) override;

    float resolutionScale = 1.0f;

    int maxSteps = 100;
    int maxBinarySearchSteps = 5;

    float rayStep = 0.02;

    LLGL::Extent2D resolution;

    PostProcessingPtr ssr;
};

struct ProceduralSkyComponent : public ComponentBase
{
    ProceduralSkyComponent(const LLGL::Extent2D& resolution);

    void Build();

    float time = 40.0f, cirrus = 0.0f, cumulus = 0.0f;
    int flip = 0;

    EnvironmentAssetPtr asset;

    std::function<void(LLGL::CommandBuffer*)> setUniforms;

    LLGL::Extent2D resolution;
    LLGL::PipelineState* pipeline{};

private:
    void DefaultTextures();
};

struct HDRISkyComponent : public ComponentBase, public EventListener
{
public:
    HDRISkyComponent(dev::TextureAssetPtr hdri, const LLGL::Extent2D& resolution);

    void Build();
    void OnEvent(Event& event) override;

    TextureAssetPtr environmentMap;
    EnvironmentAssetPtr asset;

    LLGL::Extent2D resolution;
    LLGL::PipelineState* pipelineSky{};

private:
    void SetupSkyPipeline();
    void DefaultTextures();
};

}

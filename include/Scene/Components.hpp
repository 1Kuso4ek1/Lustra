#pragma once
#include <Mesh.hpp>
#include <ImGuiManager.hpp>
#include <Camera.hpp>
#include <PostProcessing.hpp>
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

    LLGL::PipelineState* pipeline;
};

struct CameraComponent : public ComponentBase
{
    CameraComponent() : ComponentBase("CameraComponent") {}

    Camera camera;
};

struct LightComponent : public ComponentBase
{
    LightComponent() : ComponentBase("LightComponent") {}

    glm::vec3 color = { 1.0f, 1.0f, 1.0f };

    float intensity = 1.0f, cutoff = 0.0f, outerCutoff = 0.0f;
};

struct ScriptComponent : public ComponentBase
{
    ScriptComponent() : ComponentBase("ScriptComponent") {}

    std::function<void()> start;
    std::function<void(Entity self, float)> update;
};

struct ACESTonemappingComponent : public ComponentBase
{
    ACESTonemappingComponent(
        const LLGL::Extent2D& resolution = Renderer::Get().GetSwapChain()->GetResolution(),
        bool registerEvent = true
    );

    float exposure = 1.0f;

    std::shared_ptr<PostProcessing> postProcessing;

    std::function<void(LLGL::CommandBuffer*)> setUniforms;
};

struct ProceduralSkyComponent : public ComponentBase
{
    ProceduralSkyComponent();

    float time = 40.0f, cirrus = 0.0f, cumulus = 0.0f;

    // Maybe it will be better to pass pipeline as a separate component?
    LLGL::PipelineState* pipeline;

    std::function<void(LLGL::CommandBuffer*)> setUniforms;
};

struct HDRISkyComponent : public ComponentBase, EventListener
{
public:
    HDRISkyComponent(dev::TextureAssetPtr hdri, const LLGL::Extent2D& resolution);

    void OnEvent(Event& event) override;

    TextureAssetPtr environmentMap;

    LLGL::PipelineState* pipelineSky;
    LLGL::Texture* cubeMap;

private:
    void SetupConvertPipeline();
    void SetupSkyPipeline();

    void CreateCubemap(const LLGL::Extent2D& resolution);
    void CreateRenderTargets(const LLGL::Extent2D& resolution);

    void Convert();

    LLGL::PipelineState* pipelineConvert;

    std::array<LLGL::RenderTarget*, 6> renderTargets;

    std::array<glm::mat4, 6> views =
    {
        glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f))
    };

    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
};

}

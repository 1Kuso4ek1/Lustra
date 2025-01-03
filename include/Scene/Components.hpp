#pragma once
#include <Mesh.hpp>
#include <ImGuiManager.hpp>
#include <Camera.hpp>
#include <PostProcessing.hpp>
#include <TextureAsset.hpp>

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

    glm::mat4 GetTransform() const;
};

struct MeshComponent : public ComponentBase
{
    MeshComponent() : ComponentBase("MeshComponent") {}

    std::vector<MeshPtr> meshes;
};

struct MeshRendererComponent : public ComponentBase
{
    MeshRendererComponent() : ComponentBase("MeshRendererComponent") {}

    std::vector<std::shared_ptr<TextureAsset>> materials;
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
    ACESTonemappingComponent();

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

struct HDRISkyComponent : public ComponentBase
{
    HDRISkyComponent() : ComponentBase("HDRISkyComponent") {}
};

}

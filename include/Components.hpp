#pragma once
#include <TextureManager.hpp>
#include <Mesh.hpp>
#include <ImGuiManager.hpp>
#include <Camera.hpp>
#include <PostProcessing.hpp>

#include <entt/entt.hpp>

namespace dev
{

class Entity;

struct NameComponent
{
    std::string name;
};

struct TransformComponent
{
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
    glm::vec3 scale =    { 1.0f, 1.0f, 1.0f };

    glm::mat4 GetTransform() const;
};

struct MeshComponent
{
    std::vector<std::shared_ptr<Mesh>> meshes;
};

struct MeshRendererComponent
{
    std::vector<std::shared_ptr<TextureHandle>> materials;
};

struct PipelineComponent
{
    LLGL::PipelineState* pipeline;
};

struct CameraComponent
{
    Camera camera;
};

struct LightComponent
{
    glm::vec3 color = { 1.0f, 1.0f, 1.0f };

    float intensity = 1.0f, cutoff = 0.0f, outerCutoff = 0.0f;
};

struct ScriptComponent
{
    std::function<void()> start;
    std::function<void(Entity self, float)> update;
};

struct ACESTonemappingComponent
{
    ACESTonemappingComponent();

    float exposure = 1.0f;

    std::shared_ptr<PostProcessing> postProcessing;

    std::function<void(LLGL::CommandBuffer*)> setUniforms;
};

struct ProceduralSkyComponent
{
    ProceduralSkyComponent();

    float time = 40.0f, cirrus = 0.0f, cumulus = 0.0f;

    // Maybe it will be better to pass pipeline as a separate component?
    LLGL::PipelineState* pipeline;

    std::function<void(LLGL::CommandBuffer*)> setUniforms;
};

struct HDRISkyComponent
{
    HDRISkyComponent() = default;
};

}

#pragma once
#include <ComponentBase.hpp>

namespace dev
{

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

    ScriptAssetPtr script;

    std::function<void()> start;
    std::function<void(Entity self, float)> update;
};

struct RigidBodyComponent : public ComponentBase
{
    RigidBodyComponent() : ComponentBase("RigidBodyComponent") {}
    RigidBodyComponent(RigidBodyComponent&& other) : ComponentBase("RigidBodyComponent")
    {
        body = other.body;
        other.body = nullptr;
    }
    ~RigidBodyComponent()
    {
        if(body)
            PhysicsManager::Get().DestroyBody(body->GetID());
    };

    JPH::Body* body{};
};

}

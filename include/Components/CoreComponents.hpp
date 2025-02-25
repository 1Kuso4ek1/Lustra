#pragma once
#include <ComponentBase.hpp>

namespace lustra
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
    glm::vec3 scale    = { 1.0f, 1.0f, 1.0f };

    bool overridePhysics = false;

    void SetTransform(const glm::mat4& transform);
    
    glm::mat4 GetTransform() const;
};

struct MeshComponent : public ComponentBase
{
    MeshComponent()
        : ComponentBase("MeshComponent"),
          model(AssetManager::Get().Load<ModelAsset>("cube", true)) {}

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
    PipelineComponent(
        VertexShaderAssetPtr vertexShader = {},
        FragmentShaderAssetPtr fragmentShader = {}
    ) : ComponentBase("PipelineComponent"),
        vertexShader(vertexShader),
        fragmentShader(fragmentShader)
    {
        if(vertexShader && fragmentShader)
        {
            pipeline = 
                Renderer::Get().CreatePipelineState(
                    vertexShader->shader,
                    fragmentShader->shader
                );
        }
    }

    VertexShaderAssetPtr vertexShader;
    FragmentShaderAssetPtr fragmentShader;

    LLGL::PipelineState* pipeline{};
};

struct HierarchyComponent : public ComponentBase
{
    HierarchyComponent() : ComponentBase("HierarchyComponent") {}

    entt::entity parent = entt::null;
    std::vector<entt::entity> children;
};

struct CameraComponent : public ComponentBase
{
    CameraComponent() : ComponentBase("CameraComponent") {}
    CameraComponent(CameraComponent&& other)
        : ComponentBase("CameraComponent"), camera(std::move(other.camera))
    {  }

    Camera camera;

    bool active = false;
};

struct LightComponent : public ComponentBase
{
public:
    LightComponent();
    LightComponent(LightComponent&&) = default;

    void SetupShadowMap(const LLGL::Extent2D& resolution);
    void SetupProjection();

    template<class Archive>
    void save(Archive& archive) const
    {
        archive(
            CEREAL_NVP(color), CEREAL_NVP(intensity),
            CEREAL_NVP(cutoff), CEREAL_NVP(outerCutoff),
            CEREAL_NVP(bias), CEREAL_NVP(orthoExtent),
            CEREAL_NVP(shadowMap), CEREAL_NVP(orthographic),
            CEREAL_NVP(resolution)
        );
    }

    template<class Archive>
    void load(Archive& archive)
    {
        projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);

        archive(color, intensity, cutoff, outerCutoff, bias, orthoExtent, shadowMap, orthographic, resolution);

        if(shadowMap)
            SetupShadowMap(resolution);
    }

    glm::vec3 color = { 1.0f, 1.0f, 1.0f };

    float intensity = 1.0f;
    float cutoff = 0.0f, outerCutoff = 0.0f;
    float bias = 0.00001f;
    float orthoExtent = 10.0f;

    bool shadowMap = false;
    bool orthographic = false;

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
    uint32_t moduleIndex = 0;

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

    // For saving
    struct ShapeSettings
    {
        enum class Type
        {
            Empty,
            Box,
            Sphere,
            Capsule,
            Mesh
        };

        Type type = Type::Empty;

        glm::vec3 centerOfMass;
        glm::vec3 halfExtent;
        
        float radius;
        float halfHeight;

        ModelAssetPtr meshShape;
        
    } settings;
};

}

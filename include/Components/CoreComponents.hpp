#pragma once
#include <ComponentBase.hpp>
#include <ScriptManager.hpp>

namespace lustra
{

struct NameComponent final : public ComponentBase
{
    NameComponent() : ComponentBase("NameComponent") {}

    std::string name;
};

struct TransformComponent final : public ComponentBase
{
    TransformComponent() : ComponentBase("TransformComponent") {}

    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
    glm::vec3 scale    = { 1.0f, 1.0f, 1.0f };

    bool overridePhysics = false;

    void SetTransform(const glm::mat4& transform);

    glm::mat4 GetTransform() const;
};

struct MeshComponent final : public ComponentBase
{
    MeshComponent()
        : ComponentBase("MeshComponent"),
          model(AssetManager::Get().Load<ModelAsset>("cube", true)) {}

    ModelAssetPtr model;

    bool drawable = true;
};

struct MeshRendererComponent final : public ComponentBase
{
    MeshRendererComponent() : ComponentBase("MeshRendererComponent")
    {
        materials.push_back(AssetManager::Get().Load<MaterialAsset>("default", true));
    }

    std::vector<MaterialAssetPtr> materials;
};

struct PipelineComponent final : public ComponentBase, public EventListener
{
    explicit PipelineComponent(
        const VertexShaderAssetPtr& vertexShader = {},
        const FragmentShaderAssetPtr& fragmentShader = {}
    ) : ComponentBase("PipelineComponent"),
        vertexShader(vertexShader),
        fragmentShader(fragmentShader)
    {
        EventManager::Get().AddListener(Event::Type::AssetLoaded, this);

        if(vertexShader && fragmentShader)
            SetupPipeline();
    }

    PipelineComponent(PipelineComponent&& other) noexcept
        : ComponentBase("PipelineComponent"),
          vertexShader(std::move(other.vertexShader)),
          fragmentShader(std::move(other.fragmentShader)),
          pipeline(std::move(other.pipeline))
    {
        EventManager::Get().AddListener(Event::Type::AssetLoaded, this);
    }

    PipelineComponent(const PipelineComponent& other)
        : ComponentBase("PipelineComponent"),
          vertexShader(other.vertexShader),
          fragmentShader(other.fragmentShader),
          pipeline(other.pipeline)
    {
        EventManager::Get().AddListener(Event::Type::AssetLoaded, this);
    }

    ~PipelineComponent() override
    {
        EventManager::Get().RemoveListener(Event::Type::AssetLoaded, this);
    }

    void OnEvent(Event& event) override
    {
        if(event.GetType() == Event::Type::AssetLoaded) // Redundant
        {
            const auto& assetLoadedEvent = static_cast<AssetLoadedEvent&>(event);

            if(assetLoadedEvent.GetAsset() == vertexShader || assetLoadedEvent.GetAsset() == fragmentShader)
                SetupPipeline();
        }
    }

    void SetupPipeline()
    {
        pipeline =
            Renderer::Get().CreatePipelineState(
                vertexShader->shader,
                fragmentShader->shader
            );
    }

    VertexShaderAssetPtr vertexShader;
    FragmentShaderAssetPtr fragmentShader;

    LLGL::PipelineState* pipeline{};
};

struct HierarchyComponent final : public ComponentBase
{
    HierarchyComponent() : ComponentBase("HierarchyComponent") {}
    HierarchyComponent(const HierarchyComponent& other) = delete; // !
    HierarchyComponent(HierarchyComponent&& other) = default;

    entt::entity parent = entt::null;
    std::vector<entt::entity> children;
};

struct CameraComponent final : public ComponentBase
{
    CameraComponent() : ComponentBase("CameraComponent") {}
    CameraComponent(CameraComponent&& other) noexcept
        : ComponentBase("CameraComponent"), camera(std::move(other.camera))
    {}
    CameraComponent(const CameraComponent& other) = default;

    Camera camera;

    bool active = false;
};

struct LightComponent final : public ComponentBase
{
public:
    LightComponent();
    LightComponent(LightComponent&&) = default;
    LightComponent(const LightComponent&) = default;

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
    glm::mat4 projection{};

private:
    void CreateDepth(const LLGL::Extent2D& resolution);
    void CreateRenderTarget(const LLGL::Extent2D& resolution);
    void CreatePipeline();
};

struct ScriptComponent final : public ComponentBase
{
    ScriptComponent() : ComponentBase("ScriptComponent") {}
    ScriptComponent(const ScriptComponent& other) : ComponentBase("ScriptComponent")
    {
        script = other.script;
        moduleIndex = script->modulesCount++;

        ScriptManager::Get().AddScript(script);

        ScriptManager::Get().BuildModule(script, (script->path.stem().string() + std::to_string(moduleIndex)));
    }
    ScriptComponent(ScriptComponent&& other) noexcept : ComponentBase("ScriptComponent")
    {
        script = other.script;
        moduleIndex = other.moduleIndex;
    }

    ScriptAssetPtr script;
    uint32_t moduleIndex = 0;

    std::function<void()> start;
    std::function<void(Entity self, float)> update;
};

struct RigidBodyComponent final : public ComponentBase
{
    RigidBodyComponent() : ComponentBase("RigidBodyComponent") {}
    RigidBodyComponent(RigidBodyComponent&& other) noexcept : ComponentBase("RigidBodyComponent")
    {
        body = other.body;
        settings = other.settings;

        other.body = nullptr;
    }
    RigidBodyComponent(const RigidBodyComponent& other) : ComponentBase("RigidBodyComponent")
    {
        body = PhysicsManager::Get().GetBodyInterface().CreateBody(
                   other.body->GetBodyCreationSettings()
               );
        PhysicsManager::Get().GetBodyInterface().AddBody(body->GetID(), JPH::EActivation::Activate);

        settings = other.settings;
    }
    ~RigidBodyComponent() override
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

    } settings{};
};

struct SoundComponent final : public ComponentBase
{
    SoundComponent() : ComponentBase("SoundComponent") {}
    ~SoundComponent() override
    {
        if(sound)
            sound->sound.Stop();
    }

    SoundAssetPtr sound;

    bool attached = true;
};

struct PrefabComponent final : public ComponentBase
{
    PrefabComponent() : ComponentBase("PrefabComponent") {}
    PrefabComponent(const PrefabComponent& other) = delete;
    PrefabComponent(PrefabComponent&& other) = default;
};

}

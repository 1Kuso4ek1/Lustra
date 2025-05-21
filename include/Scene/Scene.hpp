#pragma once
#include <AssetManager.hpp>
#include <Components.hpp>
#include <DeferredRenderer.hpp>
#include <InputManager.hpp>
#include <Renderer.hpp>

#include <entt/entt.hpp>

namespace lustra
{

class Entity;

class Scene final : public EventListener
{
public:
    Scene();
    ~Scene() override;

    void Setup();

    void Start();

    void Update(float deltaTime);
    void Draw(LLGL::RenderTarget* renderTarget = Renderer::Get().GetSwapChain());

    void OnEvent(Event& event) override;

    void SetUpdatePhysics(bool updatePhysics);
    void ToggleUpdatePhysics();
    void SetIsRunning(bool running);
    void ToggleIsRunning();

    void ReparentEntity(Entity child, Entity parent);

    void RemoveEntity(const Entity& entity);

    Entity CreateEntity();
    Entity CloneEntity(const Entity& entity);
    Entity GetEntity(entt::id_type id);
    Entity GetEntity(const std::string& name); // Not using std::string_view since this function is used by Angelscript

    bool IsChildOf(const Entity& child, const Entity& parent);

    glm::mat4 GetWorldTransform(entt::entity entity);

    entt::registry& GetRegistry();

private:
    void StartScript(const ScriptComponent& script, const Entity& entity);
    static void UpdateScript(const ScriptComponent& script, Entity entity, float deltaTime);

    void SetupLightsBuffer();
    void SetupShadowsBuffer();
    void UpdateLightsBuffer();
    void UpdateShadowsBuffer();

    void UpdateRigidBodies();
    void UpdateSounds();

    void SetupCamera();
    void SetupLights();
    void SetupShadows();

    void RenderMeshes();
    void RenderToShadowMap();
    void RenderSky(LLGL::RenderTarget* renderTarget);

    static void MeshRenderPass(
        const MeshComponent& mesh,
        const MeshRendererComponent& meshRenderer,
        const PipelineComponent& pipeline,
        LLGL::RenderTarget* renderTarget
    );
    static void ShadowRenderPass(
        const LightComponent& light,
        const MeshComponent& mesh
    );
    static void ProceduralSkyRenderPass(
        const MeshComponent& mesh,
        const ProceduralSkyComponent& sky,
        LLGL::RenderTarget* renderTarget
    );
    static void HDRISkyRenderPass(
        const MeshComponent& mesh,
        const HDRISkyComponent& sky,
        LLGL::RenderTarget* renderTarget
    );

    void RenderResult(LLGL::RenderTarget* renderTarget = Renderer::Get().GetSwapChain());

    void ApplyPostProcessing(LLGL::RenderTarget* renderTarget);

    std::pair<LLGL::Texture*, float> ApplyBloom(LLGL::Texture* frame);
    LLGL::Texture* ApplyGTAO();
    LLGL::Texture* ApplySSR(LLGL::Texture* frame);

private:
    bool isRunning = false;
    bool updatePhysics = false;

private:
    Camera* camera{};

    glm::vec3 cameraPosition{}; // Only for shaders

private:
    struct Light
    {
        // Uniform buffer padding //
        // | //
        // V //
        alignas(16) int shadowMap;
        alignas(16) glm::vec3 position;
        alignas(16) glm::vec3 direction;
        alignas(16) glm::vec3 color;

        float intensity, cutoff, outerCutoff;
    };

    struct Shadow
    {
        glm::mat4 lightSpaceMatrix;

        alignas(16) float bias;
    };

    std::vector<Light> lights;
    std::vector<Shadow> shadows;

    std::array<LLGL::Texture*, 4> shadowSamplers{};

    LLGL::Buffer* lightsBuffer{};
    LLGL::Buffer* shadowsBuffer{};

private:
    entt::registry registry{};

private:
    friend class Entity;
};

}
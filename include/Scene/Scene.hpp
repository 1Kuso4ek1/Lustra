#pragma once
#include <Components.hpp>
#include <Renderer.hpp>
#include <DeferredRenderer.hpp>
#include <AssetManager.hpp>
#include <InputManager.hpp>

#include <entt/entt.hpp>

namespace lustra
{

class Entity;

class Scene : public EventListener
{
public:
    Scene(std::shared_ptr<RendererBase> renderer = std::make_shared<RendererBase>());
    ~Scene();

    void Setup();

    void SetRenderer(std::shared_ptr<RendererBase> renderer);

    void Start();

    void Update(float deltaTime);
    void Draw(LLGL::RenderTarget* renderTarget = Renderer::Get().GetSwapChain());

    void OnEvent(Event& event) override;

    void SetUpdatePhysics(bool updatePhysics);
    void ToggleUpdatePhysics();
    void SetIsRunning(bool running);
    void ToggleIsRunning();

    void ReparentEntity(Entity child, Entity parent);

    void RemoveEntity(Entity entity);

    Entity CreateEntity();
    Entity CloneEntity(Entity entity);
    Entity GetEntity(entt::id_type id);
    Entity GetEntity(const std::string& name); // Not using std::string_view since this function is used by Angelscript

    bool IsChildOf(Entity child, Entity parent);

    glm::mat4 GetWorldTransform(entt::entity entity);

    entt::registry& GetRegistry();

private:
    void SetupLightsBuffer();
    void SetupShadowsBuffer();
    void UpdateLightsBuffer();
    void UpdateShadowsBuffer();

    void UpdateSounds();

    void SetupCamera();
    void SetupLights();
    void SetupShadows();

    void RenderMeshes();
    void RenderToShadowMap();
    void RenderSky(LLGL::RenderTarget* renderTarget);

    void MeshRenderPass(
        const MeshComponent& mesh,
        const MeshRendererComponent& meshRenderer,
        const PipelineComponent& pipeline,
        LLGL::RenderTarget* renderTarget
    );
    void ShadowRenderPass(
        const LightComponent& light,
        const MeshComponent& mesh
    );
    void ProceduralSkyRenderPass(
        const MeshComponent& mesh,
        const ProceduralSkyComponent& sky,
        LLGL::RenderTarget* renderTarget
    );
    void HDRISkyRenderPass(
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

    glm::vec3 cameraPosition; // Only for shaders

private:
    struct Light
    {
        // Uniform buffer padding //
        // | //
        // V //
        alignas(16) glm::vec3 position;
        alignas(16) glm::vec3 direction;
        alignas(16) glm::vec3 color;
        
        float intensity, cutoff, outerCutoff;
    };

    struct Shadow
    {
        glm::mat4 lightSpaceMatrix;

        float bias __attribute__ ((aligned(16)));
    };

    std::vector<Light> lights;
    std::vector<Shadow> shadows;
    
    std::array<LLGL::Texture*, 4> shadowSamplers;

    LLGL::Buffer* lightsBuffer{};
    LLGL::Buffer* shadowsBuffer{};

private:
    std::shared_ptr<RendererBase> renderer;

private:
    entt::registry registry{};

private:
    friend class Entity;
};

}
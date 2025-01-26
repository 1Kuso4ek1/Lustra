#pragma once
#include <Components.hpp>
#include <Renderer.hpp>
#include <DeferredRenderer.hpp>
#include <AssetManager.hpp>

#include <entt/entt.hpp>

#include <unordered_map>

namespace dev
{

class Entity;

class Scene
{
public:
    Scene(std::shared_ptr<RendererBase> renderer = std::make_shared<RendererBase>());

    void SetRenderer(std::shared_ptr<RendererBase> renderer);

    void Start();

    void Update(float deltaTime);
    void Draw(LLGL::RenderTarget* renderTarget = Renderer::Get().GetSwapChain());

    void RemoveEntity(const Entity& entity);

    Entity CreateEntity();

    entt::registry& GetRegistry();

private:
    void SetupLightsBuffer();
    void SetupShadowsBuffer();
    void UpdateLightsBuffer();
    void UpdateShadowsBuffer();

    void SetupCamera();
    void SetupLights();
    void SetupShadows();

    void SetupCombinePostProcessing();

    void RenderMeshes();
    void RenderToShadowMap();
    void RenderSky(LLGL::RenderTarget* renderTarget);

    void MeshRenderPass(
        MeshComponent mesh,
        MeshRendererComponent meshRenderer,
        PipelineComponent pipeline,
        LLGL::RenderTarget* renderTarget
    );
    void ShadowRenderPass(
        LightComponent light,
        MeshComponent mesh
    );
    void ProceduralSkyRenderPass(
        MeshComponent mesh,
        ProceduralSkyComponent sky,
        LLGL::RenderTarget* renderTarget
    );
    void HDRISkyRenderPass(
        MeshComponent mesh,
        HDRISkyComponent sky,
        LLGL::RenderTarget* renderTarget
    );

    void RenderResult(LLGL::RenderTarget* renderTarget = Renderer::Get().GetSwapChain());

    void ApplyPostProcessing(LLGL::RenderTarget* renderTarget);
    
    std::pair<LLGL::Texture*, float> ApplyBloom(LLGL::Texture* frame);

private:
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
    std::shared_ptr<PostProcessing> combinePost;

private:
    entt::registry registry;

    uint64_t idCounter = 0;

    std::unordered_map<uint64_t, entt::entity> entities;

private:
    friend class Entity;
};

}
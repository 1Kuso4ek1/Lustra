#pragma once
#include <Components.hpp>
#include <Renderer.hpp>
#include <DeferredRenderer.hpp>

#include <entt/entt.hpp>

#include <unordered_map>

namespace dev
{

class Entity;

class Scene : public EventListener
{
public:
    Scene(std::shared_ptr<RendererBase> renderer = std::make_shared<RendererBase>());

    void SetRenderer(std::shared_ptr<RendererBase> renderer);

    void Start();

    void Update(float deltaTime);
    void Draw();

    void OnEvent(Event& event) override;

    Entity CreateEntity();

    entt::registry& GetRegistry();

private:
    void SetupLightsBuffer();

    void SetupCamera();
    void SetupLights();

    void RenderMeshes(MeshComponent mesh, MeshRendererComponent meshRenderer, PipelineComponent pipeline);

    void ApplyPostProcessing();

private:
    glm::vec3 cameraPosition; // Only for shaders

private:
    struct Light
    {
        glm::vec3 position __attribute__ ((aligned(16))); // Uniform buffer padding
        glm::vec3 color    __attribute__ ((aligned(16)));
        
        float intensity;
    };

    std::vector<Light> lights;

    LLGL::Buffer* lightsBuffer{};

private:
    std::shared_ptr<RendererBase> renderer;

private:
    entt::registry registry;

    uint64_t idCounter = 0;

    std::unordered_map<uint64_t, entt::entity> entities;

private:
    friend class Entity;
};

}
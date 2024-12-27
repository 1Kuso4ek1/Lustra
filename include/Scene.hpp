#pragma once
#include <Components.hpp>
#include <Renderer.hpp>
#include <DeferredRenderer.hpp>

#include <entt/entt.hpp>
#include <unordered_map>

namespace dev
{

class Entity;

class Scene
{
public:
    Scene(std::shared_ptr<RendererBase> renderer = std::make_shared<RendererBase>()) : renderer(renderer) {};

    void SetRenderer(std::shared_ptr<RendererBase> renderer);

    void Start();

    void Update(float deltaTime);
    void Draw();

    Entity CreateEntity();

    entt::registry& GetRegistry();

private:
    void SetupCamera();

    void RenderMeshes(MeshComponent mesh, MeshRendererComponent meshRenderer, PipelineComponent pipeline);

    void ApplyPostProcessing();

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
#include "Renderer.hpp"
#include <Scene.hpp>
#include <Entity.hpp>

namespace dev
{

void Scene::Update()
{
    
}

void Scene::Draw()
{
    auto view = registry.view<TransformComponent, MeshComponent, MaterialComponent, PipelineComponent>();

    Renderer::Get().Begin();

    for(auto entity : view)
    {
        auto [transform, mesh, material, pipeline] = 
                view.get<TransformComponent, MeshComponent, MaterialComponent, PipelineComponent>(entity);

        Renderer::Get().GetMatrices()->PushMatrix();
        Renderer::Get().GetMatrices()->GetModel() = transform.GetTransform();

        Renderer::Get().RenderPass(
            [&](auto commandBuffer)
            {
                mesh.meshes[0]->BindBuffers(commandBuffer);
            },
            {
                { 0, Renderer::Get().GetMatricesBuffer() },
                { 1, material.albedo[0]->texture },
                { 2, TextureManager::Get().GetAnisotropySampler() }
            },
            [&](auto commandBuffer)
            {
                mesh.meshes[0]->Draw(commandBuffer);
            },
            pipeline.pipeline
        );

        Renderer::Get().GetMatrices()->PopMatrix();
    }
    
    Renderer::Get().End();

    Renderer::Get().Submit();
}

Entity Scene::CreateEntity()
{
    Entity entity{ registry.create(), this };

    entities[idCounter++] = entity;

    return entity;
}

entt::registry& Scene::GetRegistry()
{
    return registry;
}
    
}

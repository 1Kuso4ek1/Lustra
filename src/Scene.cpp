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
    auto cameraView = registry.view<TransformComponent, CameraComponent>();
    auto view = registry.view<TransformComponent, MeshComponent, MaterialComponent, PipelineComponent>();

    Camera* cam{};
    TransformComponent cameraTransform;

    for(auto entity : cameraView)
    {
        cam = &cameraView.get<CameraComponent>(entity).camera;
        cameraTransform = cameraView.get<TransformComponent>(entity);

        break;
    }

    if(cam)
    {
        auto delta = glm::quat(cameraTransform.rotation) * glm::vec3(0.0f, 0.0f, -1.0f);
        
        Renderer::Get().GetMatrices()->GetView() = glm::lookAt(cameraTransform.position, cameraTransform.position + delta, glm::vec3(0.0f, 1.0f, 0.0f));
        Renderer::Get().GetMatrices()->GetProjection() = cam->GetProjectionMatrix();
    }

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

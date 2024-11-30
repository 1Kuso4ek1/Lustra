#include <Scene.hpp>
#include <Entity.hpp>

namespace dev
{

void Scene::Update()
{
    auto view = registry.view<TransformComponent, MeshRendererComponent>();

    for(auto entity : view)
    {
        auto [transform, mesh] = view.get<TransformComponent, MeshRendererComponent>(entity);

        // ................
    }
}

Entity Scene::CreateEntity()
{
    Entity entity{ registry.create(), this };
    entity.AddComponent<TransformComponent>();

    entities[idCounter++] = entity;

    return entity;
}
    
}

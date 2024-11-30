#include <Entity.hpp>

namespace dev
{

Entity::Entity(entt::entity entity, Scene* scene)
    : scene(scene), entity(entity) {}

template<class T, class... Args>
T& Entity::AddComponent(Args&&... args)
{
    T& component = scene->registry.emplace<T>(entity, std::forward<Args>(args)...);

    return component;
}

template<class T>
void Entity::RemoveComponent()
{
    scene->registry.remove<T>(entity);
}

template<class T>
bool Entity::HasComponent() const
{
    return scene->registry.all_of<T>(entity);
}

template<class T>
T& Entity::GetComponent()
{
    return scene->registry.get<T>(entity);
}

Entity::operator bool() const
{
    return entity != entt::null;
}

Entity::operator entt::entity() const
{
    return entity;
}

}

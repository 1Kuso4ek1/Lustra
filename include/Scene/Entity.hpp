#pragma once
#include <Scene.hpp>

namespace lustra
{

class Entity
{
public:
    Entity() = default;
    Entity(entt::entity entity, Scene* scene);

    template<class T, class... Args>
    T& AddComponent(Args&&... args)
    {
        T& component = scene->registry.emplace<T>(entity, std::forward<Args>(args)...);

        return component;
    }

    template<class T>
    void RemoveComponent() const
    {
        scene->registry.remove<T>(entity);
    }

    template<class T>
    T& GetComponent()
    {
        return scene->registry.get<T>(entity);
    }

    template<class T, class... Args>
    T& GetOrAddComponent(Args&&... args)
    {
        if(scene->registry.all_of<T>(entity))
            return scene->registry.get<T>(entity);
        return scene->registry.emplace<T>(entity, std::forward<Args>(args)...);
    }

    template<class T>
    bool HasComponent() const
    {
        return scene->registry.all_of<T>(entity);
    }

    Entity& operator=(const Entity entity)
    {
        this->entity = entity.entity;
        this->scene = entity.scene;

        return *this;
    }

    bool operator==(const Entity& other) const
    {
        return entity == other.entity && scene == other.scene;
    }

    bool operator!=(const Entity& other) const
    {
        return !(*this == other);
    }

    operator bool() const
    {
        return entity != entt::null;
    }

    operator entt::entity() const
    {
        return entity;
    }

private:
    entt::entity entity{ entt::null };

    Scene* scene{};
};

}

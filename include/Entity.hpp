#pragma once
#include <Scene.hpp>

namespace dev
{

class Entity
{
public:
    Entity() = default;
    Entity(entt::entity entity, Scene* scene);

    template<class T, class... Args>
    T& AddComponent(Args&&... args);

    template<class T>
    void RemoveComponent();

    template<class T>
    T& GetComponent();

    template<class T>
    bool HasComponent() const;

    operator bool() const;
    operator entt::entity() const;

private:
    entt::entity entity{ entt::null };

    Scene* scene{};
};

}

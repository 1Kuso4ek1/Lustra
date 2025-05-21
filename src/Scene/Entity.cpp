#include <Entity.hpp>

namespace lustra
{

Entity::Entity(const entt::entity entity, Scene* scene)
    : entity(entity), scene(scene) {}

}

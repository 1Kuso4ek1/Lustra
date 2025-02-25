#include <Entity.hpp>

namespace lustra
{

Entity::Entity(entt::entity entity, Scene* scene)
    : scene(scene), entity(entity) {}

}

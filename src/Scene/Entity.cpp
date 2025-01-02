#include <Entity.hpp>

namespace dev
{

Entity::Entity(entt::entity entity, Scene* scene)
    : scene(scene), entity(entity) {}

}

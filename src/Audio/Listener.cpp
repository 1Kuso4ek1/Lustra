#include <Listener.hpp>

#include <glm/gtc/quaternion.hpp>

namespace lustra
{

void Listener::SetPosition(const glm::vec3& position)
{
    ma_engine_listener_set_position(
        AudioManager::Get().GetEngine(), 0,
        position.x, position.y, position.z
    );
}

void Listener::SetVelocity(const glm::vec3& velocity)
{
    ma_engine_listener_set_velocity(
        AudioManager::Get().GetEngine(), 0,
        velocity.x, velocity.y, velocity.z
    );
}

void Listener::SetOrientation(const glm::quat& orientation)
{
    const auto forward = orientation * glm::vec3(0.0f, 0.0f, -1.0f);
    const auto up = orientation * glm::vec3(0.0f, 1.0f, 0.0f);

    ma_engine_listener_set_direction(
        AudioManager::Get().GetEngine(), 0,
        forward.x, forward.y, forward.z
    );

    ma_engine_listener_set_world_up(
        AudioManager::Get().GetEngine(), 0,
        up.x, up.y, up.z
    );
}

void Listener::SetDirection(const glm::vec3& direction)
{
    ma_engine_listener_set_direction(
        AudioManager::Get().GetEngine(), 0,
        direction.x, direction.y, direction.z
    );
}

void Listener::SetWorldUp(const glm::vec3& up)
{
    ma_engine_listener_set_world_up(
        AudioManager::Get().GetEngine(), 0,
        up.x, up.y, up.z
    );
}

void Listener::SetCone(const float inner, const float outer, const float outerGain)
{
    ma_engine_listener_set_cone(
        AudioManager::Get().GetEngine(), 0,
        inner, outer, outerGain
    );
}

glm::vec3 Listener::GetPosition()
{
    const auto [x, y, z] = ma_engine_listener_get_position(AudioManager::Get().GetEngine(), 0);

    return { x, y, z };
}

glm::vec3 Listener::GetVelocity()
{
    const auto [x, y, z] = ma_engine_listener_get_velocity(AudioManager::Get().GetEngine(), 0);

    return { x, y, z };
}

glm::vec3 Listener::GetDirection()
{
    const auto [x, y, z] = ma_engine_listener_get_direction(AudioManager::Get().GetEngine(), 0);

    return { x, y, z };
}

glm::vec3 Listener::GetWorldUp()
{
    const auto [x, y, z] = ma_engine_listener_get_world_up(AudioManager::Get().GetEngine(), 0);

    return { x, y, z };
}

}

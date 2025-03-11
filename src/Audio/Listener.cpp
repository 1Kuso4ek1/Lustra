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
    glm::vec3 forward = orientation * glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = orientation * glm::vec3(0.0f, 1.0f, 0.0f);

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

void Listener::SetCone(float inner, float outer, float outerGain)
{
    ma_engine_listener_set_cone(
        AudioManager::Get().GetEngine(), 0,
        inner, outer, outerGain
    );
}

glm::vec3 Listener::GetPosition()
{
    auto position = ma_engine_listener_get_position(AudioManager::Get().GetEngine(), 0);

    return { position.x, position.y, position.z };
}

glm::vec3 Listener::GetVelocity()
{
    auto velocity = ma_engine_listener_get_velocity(AudioManager::Get().GetEngine(), 0);

    return { velocity.x, velocity.y, velocity.z };
}

glm::vec3 Listener::GetDirection()
{
    auto direction = ma_engine_listener_get_direction(AudioManager::Get().GetEngine(), 0);

    return { direction.x, direction.y, direction.z };
}

glm::vec3 Listener::GetWorldUp()
{
    auto up = ma_engine_listener_get_world_up(AudioManager::Get().GetEngine(), 0);

    return { up.x, up.y, up.z };
}

}

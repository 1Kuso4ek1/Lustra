#pragma once
#include <Singleton.hpp>
#include <AudioManager.hpp>

#include <glm/glm.hpp>

namespace lustra
{

// Might not be a singleton. Pointless.
class Listener final : public Singleton<Listener>
{
public:
    static void SetPosition(const glm::vec3& position);
    static void SetVelocity(const glm::vec3& velocity);
    static void SetOrientation(const glm::quat& orientation);
    static void SetDirection(const glm::vec3& direction);
    static void SetWorldUp(const glm::vec3& up);
    static void SetCone(float inner, float outer, float outerGain); // The only thing that will be serialized in future?

    static glm::vec3 GetPosition();
    static glm::vec3 GetVelocity();
    static glm::vec3 GetDirection();
    static glm::vec3 GetWorldUp();
};

}

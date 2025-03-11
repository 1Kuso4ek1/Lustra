#pragma once
#include <Singleton.hpp>
#include <AudioManager.hpp>

#include <glm/glm.hpp>

namespace lustra
{

class Listener : public Singleton<Listener>
{
public:
    void SetPosition(const glm::vec3& position);
    void SetVelocity(const glm::vec3& velocity);
    void SetOrientation(const glm::quat& orientation);
    void SetDirection(const glm::vec3& direction);
    void SetWorldUp(const glm::vec3& up);
    void SetCone(float inner, float outer, float outerGain); // The only thing that will be serialized in future?

    glm::vec3 GetPosition();
    glm::vec3 GetVelocity();
    glm::vec3 GetDirection();
    glm::vec3 GetWorldUp();

};

}

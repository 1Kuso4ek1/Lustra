#include <Components.hpp>

namespace dev
{
    glm::mat4 TransformComponent::GetTransform() const
    {
        glm::mat4 rotation = glm::toMat4(glm::quat(rotation));

        return glm::translate(glm::mat4(1.0f), position)
                * rotation
                * glm::scale(glm::mat4(1.0f), scale);
    }
}
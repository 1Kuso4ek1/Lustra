#include <Components.hpp>

namespace dev
{
    glm::mat4 TransformComponent::GetTransform() const
    {
        return glm::translate(glm::mat4(1.0f), position)
                * glm::toMat4(rotation)
                * glm::scale(glm::mat4(1.0f), scale);
    }
}
#pragma once
#include <Mesh.hpp>

namespace dev
{

struct NameComponent
{
    std::string name;
};

struct TransformComponent
{
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

    glm::mat4 GetTransform() const;
};

struct MeshRendererComponent
{
    std::shared_ptr<Mesh> mesh;
};

}


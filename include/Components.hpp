#pragma once
#include <TextureManager.hpp>
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
    glm::quat rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
    glm::vec3 scale =    { 1.0f, 1.0f, 1.0f };

    glm::mat4 GetTransform() const;
};

struct MeshComponent
{
    std::vector<std::shared_ptr<Mesh>> meshes;
};

struct MaterialComponent
{
    std::vector<std::shared_ptr<TextureHandle>> albedo;
};

struct PipelineComponent
{
    LLGL::PipelineState* pipeline;
};

}

#pragma once
#include <Utils.hpp>

namespace lustra
{

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 coords;
};

class Mesh
{
public:
    Mesh() = default;
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, bool setupBuffers = true);

    void SetupBuffers();

    void CreateCube();
    void CreatePlane();

    void BindBuffers(LLGL::CommandBuffer* commandBuffer, bool bindMatrices = true) const;

    void Draw(LLGL::CommandBuffer* commandBuffer) const;

    std::vector<Vertex> GetVertices() const;
    std::vector<uint32_t> GetIndices() const;

private:
    void CreateVertexBuffer();
    void CreateIndexBuffer();

private:
    LLGL::VertexFormat vertexFormat;

    LLGL::Buffer* vertexBuffer{};
    LLGL::Buffer* indexBuffer{};
    LLGL::Buffer* matricesBuffer{};

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

using MeshPtr = std::shared_ptr<Mesh>;

}

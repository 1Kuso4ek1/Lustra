#include <Mesh.hpp>

namespace lustra
{

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const bool setupBuffers)
            : vertices(vertices), indices(indices)
{
    if(setupBuffers)
        SetupBuffers();
}

void Mesh::SetupBuffers()
{
    vertexFormat = Renderer::Get().GetDefaultVertexFormat();
    matricesBuffer = Renderer::Get().GetMatricesBuffer();

    CreateVertexBuffer();
    CreateIndexBuffer();
}

void Mesh::CreateCube()
{
    vertices =
    {
        { { -1, -1, -1 }, { 0, 0, -1 }, { 0, 1 } },
        { { -1, 1, -1 }, { 0, 0, -1 }, { 0, 0 } },
        { { 1, 1, -1 }, { 0, 0, -1 }, { 1, 0 } },
        { { 1, -1, -1 }, { 0, 0, -1 }, { 1, 1 } },

        { {  1, -1, -1 }, { 1, 0, 0 }, { 0, 1 } },
        { {  1, 1, -1 }, { 1, 0, 0 }, { 0, 0 } },
        { {  1, 1, 1 }, { 1, 0, 0 }, { 1, 0 } },
        { {  1, -1, 1 }, { 1, 0, 0 }, { 1, 1 } },

        { { -1, -1, 1 }, { -1, 0, 0 }, { 0, 1 } },
        { { -1, 1, 1 }, { -1, 0, 0 }, { 0, 0 } },
        { { -1, 1, -1 }, { -1, 0, 0 }, { 1, 0 } },
        { { -1, -1, -1 }, { -1, 0, 0 }, { 1, 1 } },

        { { -1, 1, -1 }, { 0, 1, 0 }, { 0, 1 } },
        { { -1, 1, 1 }, { 0, 1, 0 }, { 0, 0 } },
        { { 1, 1, 1 }, { 0, 1, 0 }, { 1, 0 } },
        { { 1, 1, -1 }, { 0, 1, 0 }, { 1, 1 } },

        { { -1, -1, 1 }, { 0, -1, 0 }, { 0, 1 } },
        { { -1, -1, -1 }, { 0, -1, 0 }, { 0, 0 } },
        { { 1, -1, -1 }, { 0, -1, 0 }, { 1, 0 } },
        { { 1, -1, 1 }, { 0, -1, 0 }, { 1, 1 } },

        { { 1, -1, 1 }, { 0, 0, 1 }, { 0, 1 } },
        { { 1, 1, 1 }, { 0, 0, 1 }, { 0, 0 } },
        { { -1, 1, 1 }, { 0, 0, 1 }, { 1, 0 } },
        { { -1, -1, 1 }, { 0, 0, 1 }, { 1, 1 } }
    };

    indices =
    {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23
    };

    SetupBuffers();
}

void Mesh::CreatePlane()
{
    vertices =
    {
        { { -1, 1, 0 }, { 0, 1, 0 }, { 0, 0 } },
        { { 1, 1, 0 }, { 0, 1, 0 }, { 1, 0 } },
        { { -1, -1, 0 }, { 0, 1, 0 }, { 0, 1 } },
        { { 1, -1, 0 }, { 0, 1, 0 }, { 1, 1 } }
    };

    indices = { 0, 1, 2, 2, 1, 3 };

    SetupBuffers();
}

void Mesh::BindBuffers(LLGL::CommandBuffer* commandBuffer, const bool bindMatrices) const
{
    commandBuffer->SetVertexBuffer(*vertexBuffer);
    commandBuffer->SetIndexBuffer(*indexBuffer);

    if(bindMatrices)
    {
        const auto matricesBinding = Renderer::Get().GetMatrices()->GetBinding();

        commandBuffer->UpdateBuffer(*matricesBuffer, 0, &matricesBinding, sizeof(Matrices::Binding));
    }
}

void Mesh::Draw(LLGL::CommandBuffer* commandBuffer) const
{
    commandBuffer->DrawIndexed(indices.size(), 0);
}

std::vector<Vertex> Mesh::GetVertices() const
{
    return vertices;
}

std::vector<uint32_t> Mesh::GetIndices() const
{
    return indices;
}

void Mesh::CreateVertexBuffer()
{
    const auto bufferDesc = LLGL::VertexBufferDesc(vertices.size() * sizeof(Vertex), vertexFormat);

    vertexBuffer = Renderer::Get().CreateBuffer(bufferDesc, vertices.data());
}

void Mesh::CreateIndexBuffer()
{
    const auto bufferDesc = LLGL::IndexBufferDesc(indices.size() * sizeof(uint32_t), LLGL::Format::R32UInt);

    indexBuffer = Renderer::Get().CreateBuffer(bufferDesc, indices.data());
}

}

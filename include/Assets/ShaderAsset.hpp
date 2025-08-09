#pragma once
#include <Asset.hpp>

#include <Renderer.hpp>

namespace lustra
{

struct VertexShaderAsset final : Asset
{
    explicit VertexShaderAsset(LLGL::Shader* shader)
        : Asset(Type::VertexShader), shader(shader) {}

    LLGL::Shader* shader{};
};

struct FragmentShaderAsset final : Asset
{
    explicit FragmentShaderAsset(LLGL::Shader* shader)
        : Asset(Type::FragmentShader), shader(shader) {}

    LLGL::Shader* shader{};
};

using VertexShaderAssetPtr = std::shared_ptr<VertexShaderAsset>;
using FragmentShaderAssetPtr = std::shared_ptr<FragmentShaderAsset>;

}

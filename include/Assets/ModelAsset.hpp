#pragma once
#include <Asset.hpp>
#include <Mesh.hpp>
#include <vector>

namespace lustra
{

struct ModelAsset final : Asset
{
    ModelAsset() : Asset(Type::Model) {};
    explicit ModelAsset(const std::vector<MeshPtr>& meshes) : Asset(Type::Model), meshes(meshes) {}

    std::vector<MeshPtr> meshes, temporaryMeshes;
};

using ModelAssetPtr = std::shared_ptr<ModelAsset>;

}

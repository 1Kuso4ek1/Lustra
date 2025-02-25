#pragma once
#include <Asset.hpp>
#include <Mesh.hpp>
#include <vector>

namespace lustra
{

struct ModelAsset : public Asset
{
    ModelAsset() : Asset(Type::Model) {};
    ModelAsset(std::vector<MeshPtr> meshes) : Asset(Type::Model), meshes(meshes) {}
    
    std::vector<MeshPtr> meshes, temporaryMeshes;
};

using ModelAssetPtr = std::shared_ptr<ModelAsset>;

}

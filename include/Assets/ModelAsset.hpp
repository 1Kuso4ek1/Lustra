#pragma once
#include <Asset.hpp>
#include <Mesh.hpp>
#include <vector>

namespace dev
{

struct ModelAsset : public Asset
{
    struct Material
    {

    };

    ModelAsset() : Asset(Type::Model) {};
    ModelAsset(
        std::vector<MeshPtr> meshes,
        std::vector<std::shared_ptr<Material>> materials = {}
    ) : Asset(Type::Model), meshes(meshes), materials(materials) {}
    
    std::vector<MeshPtr> meshes;
    std::vector<std::shared_ptr<Material>> materials;
};

}

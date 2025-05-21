#pragma once
#include <AssetLoader.hpp>
#include <ModelAsset.hpp>

#include <assimp/BaseImporter.h>
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/ai_assert.h>
#include <assimp/texture.h>
#include <assimp/Vertex.h>
#include <assimp/Bitmap.h>

namespace lustra
{

class ModelLoader final : public AssetLoader, public Singleton<ModelLoader>
{
public:
    AssetPtr Load(
        const std::filesystem::path& path,
        AssetPtr existing = nullptr,
        bool async = true
    ) override;

private:
    void LoadDefaultData();

private:
    void ImportModel(const std::filesystem::path& path, const ModelAssetPtr& modelAsset);

    void ProcessNode(const aiNode* node, const aiScene* scene, const ModelAssetPtr& modelAsset);
    void ProcessMaterial(aiMaterial* material, const ModelAssetPtr& modelAsset);

    static MeshPtr ProcessMesh(const aiMesh* mesh, const aiScene* scene);

private:
    MeshPtr cube, plane;
};

}

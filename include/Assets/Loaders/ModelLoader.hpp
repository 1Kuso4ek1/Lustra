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

namespace dev
{

class ModelLoader : public AssetLoader, public Singleton<ModelLoader>
{
public:
    AssetPtr Load(const std::filesystem::path& path) override;

private:
    void LoadDefaultData();

private:
    void ProcessNode(aiNode* node, const aiScene* scene, std::shared_ptr<ModelAsset> modelAsset);
    void ProcessMaterial(aiMaterial* material, std::shared_ptr<ModelAsset> modelAsset);
    
    MeshPtr ProcessMesh(aiMesh* mesh, const aiScene* scene);

private:
    MeshPtr cube, plane;
};

}

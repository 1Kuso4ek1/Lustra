#pragma once
#include <AssetLoader.hpp>
#include <MaterialAsset.hpp>

namespace lustra
{

class MaterialLoader : public AssetLoader, public Singleton<MaterialLoader>
{
public:
    AssetPtr Load(const std::filesystem::path& path, AssetPtr existing = nullptr) override;
    void Write(const AssetPtr& asset, const std::filesystem::path& path) override;

private:
    void LoadDefaultData();

private:
    TextureAssetPtr defaultTexture;
    MaterialAssetPtr defaultMaterial;
};

}

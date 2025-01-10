#pragma once
#include <AssetLoader.hpp>
#include <MaterialAsset.hpp>

namespace dev
{

class MaterialLoader : public AssetLoader, public Singleton<MaterialLoader>
{
public:
    AssetPtr Load(const std::filesystem::path& path) override;

private:
    void LoadDefaultData();

private:
    std::shared_ptr<MaterialAsset> defaultMaterial;
};

}

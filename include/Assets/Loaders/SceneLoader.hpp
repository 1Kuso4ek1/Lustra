#pragma once
#include <AssetLoader.hpp>
#include <SceneAsset.hpp>

namespace dev
{

class SceneLoader : public AssetLoader, public Singleton<SceneLoader>
{
public:
    AssetPtr Load(const std::filesystem::path& path) override;
    void Write(const AssetPtr& asset, const std::filesystem::path& path) override;
};

}

#pragma once
#include <AssetLoader.hpp>
#include <SoundAsset.hpp>

namespace lustra
{

class SoundLoader : public AssetLoader, public Singleton<SoundLoader>
{
public:
    AssetPtr Load(const std::filesystem::path& path, AssetPtr existing = nullptr) override;
};

}

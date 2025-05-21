#pragma once
#include <AssetLoader.hpp>
#include <SoundAsset.hpp>

namespace lustra
{

class SoundLoader final : public AssetLoader, public Singleton<SoundLoader>
{
public:
    AssetPtr Load(
        const std::filesystem::path& path,
        AssetPtr existing = nullptr,
        bool async = true
    ) override;
};

}

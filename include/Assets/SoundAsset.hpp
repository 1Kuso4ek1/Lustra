#pragma once
#include <Asset.hpp>
#include <AudioManager.hpp>

namespace lustra
{

struct SoundAsset final : public Asset
{
    SoundAsset() : Asset(Asset::Type::Sound) {}
    explicit SoundAsset(const Sound& sound)
        : Asset(Asset::Type::Sound), sound(sound)
    {}

    Sound sound;
};

using SoundAssetPtr = std::shared_ptr<SoundAsset>;

}

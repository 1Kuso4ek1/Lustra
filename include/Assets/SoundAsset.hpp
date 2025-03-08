#pragma once
#include <Asset.hpp>
#include <AudioManager.hpp>

namespace lustra
{

struct SoundAsset : public Asset
{
    SoundAsset() : Asset(Asset::Type::Sound) {}
    SoundAsset(const Sound& sound)
        : Asset(Asset::Type::Sound), sound(sound)
    {}

    Sound sound;
};

using SoundAssetPtr = std::shared_ptr<SoundAsset>;

}

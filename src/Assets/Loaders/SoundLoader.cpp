#include <SoundLoader.hpp>

namespace lustra
{

AssetPtr SoundLoader::Load(const std::filesystem::path& path, AssetPtr existing)
{
    auto soundAsset = existing
        ? std::static_pointer_cast<SoundAsset>(existing)
        : std::make_shared<SoundAsset>();

    soundAsset->sound = AudioManager::Get().LoadSound(path);

    soundAsset->loaded = true;

    return soundAsset;
}

}

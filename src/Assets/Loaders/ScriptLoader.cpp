#include <ScriptLoader.hpp>
#include <EventManager.hpp>

namespace lustra
{

AssetPtr ScriptLoader::Load(const std::filesystem::path& path, AssetPtr existing)
{
    auto asset = existing
        ? std::static_pointer_cast<ScriptAsset>(existing)
        : std::make_shared<ScriptAsset>();

    asset->loaded = true;

    EventManager::Get().Dispatch(std::make_unique<AssetLoadedEvent>(asset));

    return asset;
}

}

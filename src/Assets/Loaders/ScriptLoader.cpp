#include <ScriptLoader.hpp>

namespace lustra
{

AssetPtr ScriptLoader::Load(const std::filesystem::path& path)
{
    auto asset = std::make_shared<ScriptAsset>();
    asset->loaded = true;

    return asset;
}

}

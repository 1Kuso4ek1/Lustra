#include <Serialize.hpp>
#include <SceneLoader.hpp>

#include <cereal/types/string.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>

#include <fstream>

namespace lustra
{

AssetPtr SceneLoader::Load(
    const std::filesystem::path& path,
    const AssetPtr existing,
    bool async
)
{
    auto asset = existing
        ? std::static_pointer_cast<SceneAsset>(existing)
        : std::make_shared<SceneAsset>(std::make_shared<Scene>());

    if(existing)
        asset->scene->GetRegistry().clear<>();

    const bool binaryFile = path.extension() == ".scn";

    std::ifstream file(path, binaryFile ? std::ios::binary : std::ios::in);

    if(binaryFile)
    {
        cereal::BinaryInputArchive binary(file);
        Load(binary, asset);
    }
    else
    {
        cereal::JSONInputArchive json(file);
        Load(json, asset);
    }

    asset->loaded = true;

    EventManager::Get().Dispatch(std::make_unique<AssetLoadedEvent>(asset));

    LLGL::Log::Printf(
        LLGL::Log::ColorFlags::Bold | LLGL::Log::ColorFlags::Green,
        "Scene \"%s\" loaded.\n",
        path.string().c_str()
    );

    return asset;
}

void SceneLoader::Write(const AssetPtr& asset, const std::filesystem::path& path)
{
    const auto sceneAsset = std::static_pointer_cast<SceneAsset>(asset);

    const bool binaryFile = path.extension() == ".scn";

    std::ofstream file(path, binaryFile ? std::ios::binary : std::ios::out);

    if(binaryFile)
    {
        cereal::BinaryOutputArchive binary(file);
        Write(binary, sceneAsset);
    }
    else
    {
        cereal::JSONOutputArchive json(file);
        Write(json, sceneAsset);
    }
}

}

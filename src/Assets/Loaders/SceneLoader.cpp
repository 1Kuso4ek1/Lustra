#include <Serialize.hpp>
#include <SceneLoader.hpp>

#include <cereal/archives/json.hpp>

#include <fstream>

namespace dev
{

AssetPtr SceneLoader::Load(const std::filesystem::path& path)
{
    auto asset = std::make_shared<SceneAsset>(std::make_shared<Scene>());

    std::ifstream file(path);

    cereal::JSONInputArchive archive(file);

    entt::snapshot_loader(asset->scene->GetRegistry())
        .get<dev::NameComponent>(archive)
        .get<dev::MeshComponent>(archive)
        .get<dev::MeshRendererComponent>(archive)
        .get<dev::TransformComponent>(archive)
        .get<dev::PipelineComponent>(archive)
        .get<dev::HierarchyComponent>(archive)
        .get<dev::CameraComponent>(archive)
        .get<dev::LightComponent>(archive)
        .get<dev::HierarchyComponent>(archive)
        .get<dev::TonemapComponent>(archive)
        .get<dev::BloomComponent>(archive)
        .get<dev::GTAOComponent>(archive)
        .get<dev::SSRComponent>(archive)
        .get<dev::ProceduralSkyComponent>(archive)
        .get<dev::HDRISkyComponent>(archive)
        .get<dev::RigidBodyComponent>(archive);

    asset->loaded = true;
    asset->path = path;

    LLGL::Log::Printf(
        LLGL::Log::ColorFlags::Bold | LLGL::Log::ColorFlags::Green,
        "Scene \"%s\" loaded.\n",
        path.string().c_str()
    );

    return asset;
}

void SceneLoader::Write(const AssetPtr& asset, const std::filesystem::path& path)
{
    auto sceneAsset = std::static_pointer_cast<SceneAsset>(asset);

    std::ofstream file(path);

    cereal::JSONOutputArchive archive(file);

    entt::snapshot(sceneAsset->scene->GetRegistry())
        .get<dev::NameComponent>(archive)
        .get<dev::MeshComponent>(archive)
        .get<dev::MeshRendererComponent>(archive)
        .get<dev::TransformComponent>(archive)
        .get<dev::PipelineComponent>(archive)
        .get<dev::HierarchyComponent>(archive)
        .get<dev::CameraComponent>(archive)
        .get<dev::LightComponent>(archive)
        .get<dev::HierarchyComponent>(archive)
        .get<dev::TonemapComponent>(archive)
        .get<dev::BloomComponent>(archive)
        .get<dev::GTAOComponent>(archive)
        .get<dev::SSRComponent>(archive)
        .get<dev::ProceduralSkyComponent>(archive)
        .get<dev::HDRISkyComponent>(archive)
        .get<dev::RigidBodyComponent>(archive);
}

}

#pragma once
#include <AssetLoader.hpp>
#include <SceneAsset.hpp>

namespace lustra
{

class SceneLoader final : public AssetLoader, public Singleton<SceneLoader>
{
public:
    AssetPtr Load(
        const std::filesystem::path& path,
        AssetPtr existing = nullptr,
        bool async = true
    ) override;
    void Write(const AssetPtr& asset, const std::filesystem::path& path) override;

private:
    template<class Archive>
    static void Load(Archive& archive, SceneAssetPtr& asset)
    {
        entt::snapshot_loader(asset->scene->GetRegistry())
            .get<NameComponent>(archive)
            .template get<MeshComponent>(archive)
            .template get<MeshRendererComponent>(archive)
            .template get<TransformComponent>(archive)
            .template get<PipelineComponent>(archive)
            .template get<HierarchyComponent>(archive)
            .template get<CameraComponent>(archive)
            .template get<LightComponent>(archive)
            .template get<ScriptComponent>(archive)
            .template get<TonemapComponent>(archive)
            .template get<BloomComponent>(archive)
            .template get<GTAOComponent>(archive)
            .template get<SSRComponent>(archive)
            .template get<ProceduralSkyComponent>(archive)
            .template get<HDRISkyComponent>(archive)
            .template get<RigidBodyComponent>(archive)
            .template get<SoundComponent>(archive)
            .template get<PrefabComponent>(archive);

        ScriptManager::Get().Build();
    }

    template<class Archive>
    static void Write(Archive& archive, const SceneAssetPtr& asset)
    {
        entt::snapshot(asset->scene->GetRegistry())
            .get<NameComponent>(archive)
            .template get<MeshComponent>(archive)
            .template get<MeshRendererComponent>(archive)
            .template get<TransformComponent>(archive)
            .template get<PipelineComponent>(archive)
            .template get<HierarchyComponent>(archive)
            .template get<CameraComponent>(archive)
            .template get<LightComponent>(archive)
            .template get<ScriptComponent>(archive)
            .template get<TonemapComponent>(archive)
            .template get<BloomComponent>(archive)
            .template get<GTAOComponent>(archive)
            .template get<SSRComponent>(archive)
            .template get<ProceduralSkyComponent>(archive)
            .template get<HDRISkyComponent>(archive)
            .template get<RigidBodyComponent>(archive)
            .template get<SoundComponent>(archive)
            .template get<PrefabComponent>(archive);
    }
};

}

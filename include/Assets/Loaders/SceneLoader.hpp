#pragma once
#include <AssetLoader.hpp>
#include <SceneAsset.hpp>

namespace lustra
{

class SceneLoader : public AssetLoader, public Singleton<SceneLoader>
{
public:
    AssetPtr Load(const std::filesystem::path& path, AssetPtr existing = nullptr) override;
    void Write(const AssetPtr& asset, const std::filesystem::path& path) override;

private:
    template<class Archive>
    void Load(Archive& archive, SceneAssetPtr& asset)
    {
        entt::snapshot_loader(asset->scene->GetRegistry())
            .template get<NameComponent>(archive)
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
            .template get<RigidBodyComponent>(archive);
    }

    template<class Archive>
    void Write(Archive& archive, const SceneAssetPtr& asset)
    {
        entt::snapshot(asset->scene->GetRegistry())
            .template get<NameComponent>(archive)
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
            .template get<RigidBodyComponent>(archive);
    }
};

}

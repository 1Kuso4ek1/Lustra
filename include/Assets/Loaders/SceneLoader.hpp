#pragma once
#include <AssetLoader.hpp>
#include <SceneAsset.hpp>

namespace dev
{

class SceneLoader : public AssetLoader, public Singleton<SceneLoader>
{
public:
    AssetPtr Load(const std::filesystem::path& path) override;
    void Write(const AssetPtr& asset, const std::filesystem::path& path) override;

private:
    template<class Archive>
    void Load(Archive& archive, SceneAssetPtr& asset)
    {
        entt::snapshot_loader(asset->scene->GetRegistry())
            .template get<dev::NameComponent>(archive)
            .template get<dev::MeshComponent>(archive)
            .template get<dev::MeshRendererComponent>(archive)
            .template get<dev::TransformComponent>(archive)
            .template get<dev::PipelineComponent>(archive)
            .template get<dev::HierarchyComponent>(archive)
            .template get<dev::CameraComponent>(archive)
            .template get<dev::LightComponent>(archive)
            .template get<dev::ScriptComponent>(archive)
            .template get<dev::TonemapComponent>(archive)
            .template get<dev::BloomComponent>(archive)
            .template get<dev::GTAOComponent>(archive)
            .template get<dev::SSRComponent>(archive)
            .template get<dev::ProceduralSkyComponent>(archive)
            .template get<dev::HDRISkyComponent>(archive)
            .template get<dev::RigidBodyComponent>(archive);
    }

    template<class Archive>
    void Write(Archive& archive, const SceneAssetPtr& asset)
    {
        entt::snapshot(asset->scene->GetRegistry())
            .template get<dev::NameComponent>(archive)
            .template get<dev::MeshComponent>(archive)
            .template get<dev::MeshRendererComponent>(archive)
            .template get<dev::TransformComponent>(archive)
            .template get<dev::PipelineComponent>(archive)
            .template get<dev::HierarchyComponent>(archive)
            .template get<dev::CameraComponent>(archive)
            .template get<dev::LightComponent>(archive)
            .template get<dev::ScriptComponent>(archive)
            .template get<dev::TonemapComponent>(archive)
            .template get<dev::BloomComponent>(archive)
            .template get<dev::GTAOComponent>(archive)
            .template get<dev::SSRComponent>(archive)
            .template get<dev::ProceduralSkyComponent>(archive)
            .template get<dev::HDRISkyComponent>(archive)
            .template get<dev::RigidBodyComponent>(archive);
    }
};

}

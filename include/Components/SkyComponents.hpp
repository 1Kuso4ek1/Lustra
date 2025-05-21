#pragma once
#include <ComponentBase.hpp>

namespace lustra
{

struct ProceduralSkyComponent : public ComponentBase
{
    ProceduralSkyComponent(const LLGL::Extent2D& resolution = { 1024, 1024 });
    ProceduralSkyComponent(ProceduralSkyComponent&&);

    void Build();

    void MakeSetUniforms();

    template<class Archive>
    void save(Archive& archive) const
    {
        archive(
            CEREAL_NVP(time), CEREAL_NVP(cirrus),
            CEREAL_NVP(cumulus), CEREAL_NVP(resolution)
        );
    }

    template<class Archive>
    void load(Archive& archive)
    {
        archive(time, cirrus, cumulus, resolution);

        Build();
    }

    float time = 40.0f, cirrus = 0.0f, cumulus = 0.0f;
    int flip = 0;

    EnvironmentAssetPtr asset;

    std::function<void(LLGL::CommandBuffer*)> setUniforms;

    LLGL::Extent2D resolution;
    LLGL::PipelineState* pipeline{};

private:
    void DefaultTextures() const;
};

struct HDRISkyComponent : public ComponentBase, public EventListener
{
public:
    HDRISkyComponent(
        const TextureAssetPtr& hdri = AssetManager::Get().Load<TextureAsset>("empty", true),
        const LLGL::Extent2D& resolution = { 1024, 1024 }
    );
    HDRISkyComponent(HDRISkyComponent&& other);
    ~HDRISkyComponent();

    void Build();
    void OnEvent(Event& event) override;

    template<class Archive>
    void save(Archive& archive) const
    {
        archive(
            cereal::make_nvp("environmentMapPath", environmentMap->path.string()),
            CEREAL_NVP(resolution)
        );
    }

    template<class Archive>
    void load(Archive& archive)
    {
        std::string path;
        archive(path, resolution);

        environmentMap = AssetManager::Get().Load<TextureAsset>(path);

        Build();
    }

    TextureAssetPtr environmentMap;
    EnvironmentAssetPtr asset;

    LLGL::Extent2D resolution;
    LLGL::PipelineState* pipelineSky{};

private:
    void SetupSkyPipeline();
    void DefaultTextures() const;
};

}

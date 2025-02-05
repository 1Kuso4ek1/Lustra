#pragma once
#include <ComponentBase.hpp>

namespace dev
{

struct ProceduralSkyComponent : public ComponentBase
{
    ProceduralSkyComponent(const LLGL::Extent2D& resolution);

    void Build();

    float time = 40.0f, cirrus = 0.0f, cumulus = 0.0f;
    int flip = 0;

    EnvironmentAssetPtr asset;

    std::function<void(LLGL::CommandBuffer*)> setUniforms;

    LLGL::Extent2D resolution;
    LLGL::PipelineState* pipeline{};

private:
    void DefaultTextures();
};

struct HDRISkyComponent : public ComponentBase, public EventListener
{
public:
    HDRISkyComponent(dev::TextureAssetPtr hdri, const LLGL::Extent2D& resolution);

    void Build();
    void OnEvent(Event& event) override;

    TextureAssetPtr environmentMap;
    EnvironmentAssetPtr asset;

    LLGL::Extent2D resolution;
    LLGL::PipelineState* pipelineSky{};

private:
    void SetupSkyPipeline();
    void DefaultTextures();
};

}

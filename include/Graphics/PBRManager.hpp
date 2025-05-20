#pragma once
#include <AssetManager.hpp>
#include <EnvironmentAsset.hpp>
#include <EventManager.hpp>
#include <ModelAsset.hpp>
#include <TextureAsset.hpp>

namespace lustra
{

// Needs some improvements
class PBRManager final : public Singleton<PBRManager>
{
public:
    EnvironmentAssetPtr Build(
        const LLGL::Extent2D& resolution,
        const TextureAssetPtr& environmentMap,
        EnvironmentAssetPtr environmentAsset = nullptr,
        LLGL::PipelineState* customConvertPipeline = nullptr,
        const std::function<void(LLGL::CommandBuffer*)>& setConvertUniforms = nullptr
    );

private: // Singleton-related
    PBRManager();

    friend class Singleton<PBRManager>;

private:
    void RenderCubeMap(
        const std::unordered_map<uint32_t, LLGL::Resource*>& resources,
        LLGL::Texture* cubeMap,
        LLGL::PipelineState* pipeline,
        std::function<void(LLGL::CommandBuffer*)> setUniforms = nullptr
    );

    void RenderCubeMapMips(
        const std::unordered_map<uint32_t, LLGL::Resource*>& resources,
        LLGL::Texture* cubeMap,
        LLGL::PipelineState* pipeline
    );

    void RenderBRDF();

    void SetupConvertPipeline();
    void SetupIrradiancePipeline();
    void SetupPrefilteredPipeline();
    void SetupBRDFPipeline();

    void CreateCubemaps(const LLGL::Extent2D& resolution);
    void CreateRenderTargets(const LLGL::Extent2D& resolution, LLGL::Texture* cubeMap, int mipLevel = 0);

    void CreateBRDFTexture(const LLGL::Extent2D& resolution);
    void CreateBRDFRenderTarget(const LLGL::Extent2D& resolution);

    void ReleaseCubeMaps() const;
    void ReleaseRenderTargets() const;

private:
    LLGL::Texture* cubeMap{};
    LLGL::Texture* irradiance{};
    LLGL::Texture* prefiltered{};
    LLGL::Texture* brdf{};

    LLGL::PipelineState* pipelineConvert{};
    LLGL::PipelineState* pipelineIrradiance{};
    LLGL::PipelineState* pipelinePrefiltered{};
    LLGL::PipelineState* pipelineBRDF{};

    LLGL::RenderTarget* brdfRenderTarget{};

    std::array<LLGL::RenderTarget*, 6> renderTargets;

    std::array<glm::mat4, 6> views =
    {
        glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f))
    };

    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
};

}

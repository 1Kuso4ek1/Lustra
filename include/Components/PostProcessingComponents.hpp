#pragma once
#include <ComponentBase.hpp>

namespace lustra
{

struct TonemapComponent final : public ComponentBase, public EventListener
{
    explicit TonemapComponent(const LLGL::Extent2D& resolution = Renderer::Get().GetViewportResolution());
    // All these move constructors are required since:
    // 1. they're being used by entt::snapshot
    // 1.1. destructor removes listener
    // 1.2. setUniforms captures everything by reference and cannot be copied/moved
    // 1.3. I can't even think of a better way to do it...
    TonemapComponent(TonemapComponent&& other) noexcept;
    ~TonemapComponent() override;

    void SetupPostProcessing();
    void OnEvent(Event& event) override;

    void MakeSetUniforms();

    template<class Archive>
    void save(Archive& archive) const
    {
        archive(
            CEREAL_NVP(algorithm), CEREAL_NVP(exposure), CEREAL_NVP(colorGrading),
            CEREAL_NVP(colorGradingIntensity), CEREAL_NVP(vignetteIntensity), CEREAL_NVP(vignetteRoundness),
            CEREAL_NVP(filmGrain), CEREAL_NVP(contrast), CEREAL_NVP(saturation), CEREAL_NVP(brightness),
            cereal::make_nvp("lutPath", lut->path.string()), CEREAL_NVP(resolution)
        );
    }

    template<class Archive>
    void load(Archive& archive)
    {
        std::string lutPath;
        archive(
            algorithm, exposure, colorGrading, colorGradingIntensity, vignetteIntensity,
            vignetteRoundness, filmGrain, contrast, saturation, brightness, lutPath, resolution
        );

        lut = AssetManager::Get().Load<TextureAsset>(lutPath);

        SetupPostProcessing();
    }

    int algorithm = 0;
    float exposure = 1.0f;

    glm::vec3 colorGrading = { 0.0f, 0.0f, 0.0f };

    float colorGradingIntensity = 0.0f;
    float vignetteIntensity = 0.0f;
    float vignetteRoundness = 0.0f;
    float filmGrain = 0.0f;
    float contrast = 1.0f;
    float saturation = 1.0f;
    float brightness = 1.0f;

    LLGL::Extent2D resolution;

    TextureAssetPtr lut;

    PostProcessingPtr postProcessing;

    std::function<void(LLGL::CommandBuffer*)> setUniforms;
};

struct BloomComponent final : public ComponentBase, public EventListener
{
    explicit BloomComponent(const LLGL::Extent2D& resolution = Renderer::Get().GetViewportResolution());
    BloomComponent(BloomComponent&& other) noexcept;
    ~BloomComponent() override;

    void SetupPostProcessing() const;
    void OnEvent(Event& event) override;

    template<class Archive>
    void save(Archive& archive) const
    {
        archive(
            CEREAL_NVP(threshold), CEREAL_NVP(strength),
            CEREAL_NVP(resolutionScale), CEREAL_NVP(resolution)
        );
    }

    template<class Archive>
    void load(Archive& archive)
    {
        archive(threshold, strength, resolutionScale, resolution);

        SetupPostProcessing();
    }

    float threshold = 1.0f, strength = 0.3f, resolutionScale = 8.0f;

    LLGL::Extent2D resolution;

    LLGL::Sampler* sampler;

    PostProcessingPtr thresholdPass;

    std::array<PostProcessingPtr, 2> pingPong;

    std::function<void(LLGL::CommandBuffer*)> setThresholdUniforms;
};

struct GTAOComponent final : public ComponentBase, public EventListener
{
    explicit GTAOComponent(const LLGL::Extent2D& resolution = Renderer::Get().GetViewportResolution());
    GTAOComponent(GTAOComponent&& other) noexcept;
    ~GTAOComponent() override;

    void SetupPostProcessing() const;
    void OnEvent(Event& event) override;

    template<class Archive>
    void save(Archive& archive) const
    {
        archive(
            CEREAL_NVP(resolutionScale), CEREAL_NVP(samples), CEREAL_NVP(limit),
            CEREAL_NVP(radius), CEREAL_NVP(falloff), CEREAL_NVP(thicknessMix),
            CEREAL_NVP(maxStride), CEREAL_NVP(resolution)
        );
    }

    template<class Archive>
    void load(Archive& archive)
    {
        archive(resolutionScale, samples, limit, radius, falloff, thicknessMix, maxStride, resolution);

        SetupPostProcessing();
    }

    float resolutionScale = 2.0f;

    int samples = 4.0f;

    float limit = 100.0f;
    float radius = 2.0f;
    float falloff = 1.5f;
    float thicknessMix = 0.2f;
    float maxStride = 8.0f;

    LLGL::Extent2D resolution;

    PostProcessingPtr gtao, boxBlur;
};

struct SSRComponent final : public ComponentBase, public EventListener
{
    explicit SSRComponent(const LLGL::Extent2D& resolution = Renderer::Get().GetViewportResolution());
    SSRComponent(SSRComponent&& other) noexcept;
    ~SSRComponent() override;

    void SetupPostProcessing();
    void OnEvent(Event& event) override;

    template<class Archive>
    void save(Archive& archive) const
    {
        archive(
            CEREAL_NVP(resolutionScale), CEREAL_NVP(maxSteps),
            CEREAL_NVP(maxBinarySearchSteps), CEREAL_NVP(rayStep), CEREAL_NVP(resolution)
        );
    }

    template<class Archive>
    void load(Archive& archive)
    {
        archive(resolutionScale, maxSteps, maxBinarySearchSteps, rayStep, resolution);

        SetupPostProcessing();
    }

    float resolutionScale = 1.0f;

    int maxSteps = 100;
    int maxBinarySearchSteps = 5;

    float rayStep = 0.02;

    LLGL::Extent2D resolution;

    PostProcessingPtr ssr;
};

}

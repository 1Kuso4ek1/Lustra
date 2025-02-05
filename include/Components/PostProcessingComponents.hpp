#pragma once
#include <ComponentBase.hpp>

namespace dev
{

struct TonemapComponent : public ComponentBase
{
    TonemapComponent(
        const LLGL::Extent2D& resolution = Renderer::Get().GetSwapChain()->GetResolution(),
        bool registerEvent = true
    );

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

    PostProcessingPtr postProcessing;

    std::function<void(LLGL::CommandBuffer*)> setUniforms;
};

struct BloomComponent : public ComponentBase, public EventListener
{
    BloomComponent(const LLGL::Extent2D& resolution);
    ~BloomComponent();

    void SetupPostProcessing();
    void OnEvent(Event& event) override;

    float threshold = 1.0f, strength = 0.3f, resolutionScale = 8.0f;

    LLGL::Extent2D resolution;

    LLGL::Sampler* sampler;

    PostProcessingPtr thresholdPass;

    std::array<PostProcessingPtr, 2> pingPong;

    std::function<void(LLGL::CommandBuffer*)> setThresholdUniforms;
};

struct GTAOComponent : public ComponentBase, public EventListener
{
    GTAOComponent(const LLGL::Extent2D& resolution);
    ~GTAOComponent();

    void SetupPostProcessing();
    void OnEvent(Event& event) override;

    float resolutionScale = 2.0f;

    int samples = 4.0f;
    
    float limit = 100.0f;
    float radius = 8.0f;
    float falloff = 1.5f;
    float thicknessMix = 0.2f;
    float maxStride = 8.0f;

    LLGL::Extent2D resolution;

    PostProcessingPtr gtao, boxBlur;
};

struct SSRComponent : public ComponentBase, public EventListener
{
    SSRComponent(const LLGL::Extent2D& resolution);
    ~SSRComponent();

    void SetupPostProcessing();
    void OnEvent(Event& event) override;

    float resolutionScale = 1.0f;

    int maxSteps = 100;
    int maxBinarySearchSteps = 5;

    float rayStep = 0.02;

    LLGL::Extent2D resolution;

    PostProcessingPtr ssr;
};

}

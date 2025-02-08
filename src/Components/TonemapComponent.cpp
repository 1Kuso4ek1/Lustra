#include <PostProcessingComponents.hpp>

namespace dev
{

TonemapComponent::TonemapComponent(
    const LLGL::Extent2D& resolution,
    bool registerEvent
) : ComponentBase("TonemapComponent")
{
    postProcessing = std::make_shared<PostProcessing>(
        LLGL::PipelineLayoutDescriptor
        {
            .bindings =
            {
                { "frame", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 1 },
                { "bloom", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 2 },
                { "ssr", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 3 },
                { "gAlbedo", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 4 },
                { "gCombined", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 5 },
                { "lut", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 6 }
            },
            .uniforms
            {
                { "algorithm", LLGL::UniformType::Int1 },
                { "exposure", LLGL::UniformType::Float1 },
                { "bloomStrength", LLGL::UniformType::Float1 },
                { "colorGrading", LLGL::UniformType::Float3 },
                { "colorGradingIntensity", LLGL::UniformType::Float1 },
                { "vignetteIntensity", LLGL::UniformType::Float1},
                { "vignetteRoundness", LLGL::UniformType::Float1 },
                { "filmGrain", LLGL::UniformType::Float1 },
                { "contrast", LLGL::UniformType::Float1 },
                { "saturation", LLGL::UniformType::Float1 },
                { "brightness", LLGL::UniformType::Float1 },
                { "time", LLGL::UniformType::Float1 }
            }
        },
        LLGL::GraphicsPipelineDescriptor
        {
            .vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/screenRect.vert"),
            .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/tonemap.frag")
        },
        resolution,
        true,
        registerEvent
    );

    setUniforms = [&](auto commandBuffer)
    {
        auto ns = std::chrono::steady_clock::now()
              .time_since_epoch()
              .count();

        float time = (float)(ns % 1000000) / 1000000.0f;

        commandBuffer->SetUniforms(0, &algorithm, sizeof(algorithm));
        commandBuffer->SetUniforms(1, &exposure, sizeof(exposure));
        commandBuffer->SetUniforms(3, &colorGrading, sizeof(colorGrading));
        commandBuffer->SetUniforms(4, &colorGradingIntensity, sizeof(colorGradingIntensity));
        commandBuffer->SetUniforms(5, &vignetteIntensity, sizeof(vignetteIntensity));
        commandBuffer->SetUniforms(6, &vignetteRoundness, sizeof(vignetteRoundness));
        commandBuffer->SetUniforms(7, &filmGrain, sizeof(filmGrain));
        commandBuffer->SetUniforms(8, &contrast, sizeof(contrast));
        commandBuffer->SetUniforms(9, &saturation, sizeof(saturation));
        commandBuffer->SetUniforms(10, &brightness, sizeof(brightness));
        commandBuffer->SetUniforms(11, &time, sizeof(time));
    };

    lut = AssetManager::Get().Load<TextureAsset>("empty");
}

}

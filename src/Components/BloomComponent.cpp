#include <PostProcessingComponents.hpp>

namespace lustra
{

BloomComponent::BloomComponent(const LLGL::Extent2D& resolution)
    : ComponentBase("BloomComponent"), resolution(resolution)
{
    EventManager::Get().AddListener(Event::Type::WindowResize, this);

    const LLGL::Extent2D scaledResolution =
    {
        static_cast<uint32_t>(static_cast<float>(resolution.width) / resolutionScale),
        static_cast<uint32_t>(static_cast<float>(resolution.height) / resolutionScale)
    };

    LLGL::PipelineLayoutDescriptor pingPongLayout =
    {
        .bindings =
        {
            { "frame", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 1 },
            { "samplerState", LLGL::ResourceType::Sampler, 0, LLGL::StageFlags::FragmentStage, 1 }
        },
        .uniforms =
        {
            { "horizontal", LLGL::UniformType::Bool1 },
        }
    };

    thresholdPass = std::make_shared<PostProcessing>(
        LLGL::PipelineLayoutDescriptor
        {
            .bindings =
            {
                { "frame", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 1 }
            },
            .uniforms =
            {
                { "threshold", LLGL::UniformType::Float1 },
            }
        },
        AssetManager::Get().Load<VertexShaderAsset>("screenRect.vert", true),
        AssetManager::Get().Load<FragmentShaderAsset>("threshold.frag", true),
        scaledResolution,
        true,
        false
    );

    pingPong[0] = std::make_shared<PostProcessing>(
        pingPongLayout,
        AssetManager::Get().Load<VertexShaderAsset>("screenRect.vert", true),
        AssetManager::Get().Load<FragmentShaderAsset>("blur.frag", true),
        scaledResolution,
        true,
        false
    );

    pingPong[1] = std::make_shared<PostProcessing>(
        pingPongLayout,
        AssetManager::Get().Load<VertexShaderAsset>("screenRect.vert", true),
        AssetManager::Get().Load<FragmentShaderAsset>("blur.frag", true),
        scaledResolution,
        true,
        false
    );

    sampler = Renderer::Get().CreateSampler(
        {
            .addressModeU = LLGL::SamplerAddressMode::Clamp,
            .addressModeV = LLGL::SamplerAddressMode::Clamp,
            .addressModeW = LLGL::SamplerAddressMode::Clamp
        }
    );

    setThresholdUniforms = [&](auto commandBuffer)
    {
        commandBuffer->SetUniforms(0, &threshold, sizeof(threshold));
    };
}

BloomComponent::BloomComponent(BloomComponent&& other) noexcept
    : ComponentBase("BloomComponent"),
      threshold(other.threshold), strength(other.strength), resolutionScale(other.resolutionScale),
      resolution(other.resolution), sampler(other.sampler), thresholdPass(std::move(other.thresholdPass)),
      pingPong(std::move(other.pingPong))
{
    EventManager::Get().AddListener(Event::Type::WindowResize, this);

    setThresholdUniforms = [&](auto commandBuffer)
    {
        commandBuffer->SetUniforms(0, &threshold, sizeof(threshold));
    };
}

BloomComponent::~BloomComponent()
{
    EventManager::Get().RemoveListener(Event::Type::WindowResize, this);
}

void BloomComponent::SetupPostProcessing() const
{
    const LLGL::Extent2D scaledResolution =
    {
        static_cast<uint32_t>(static_cast<float>(resolution.width) / resolutionScale),
        static_cast<uint32_t>(static_cast<float>(resolution.height) / resolutionScale)
    };

    WindowResizeEvent newEvent(scaledResolution);

    thresholdPass->OnEvent(newEvent);
    pingPong[0]->OnEvent(newEvent);
    pingPong[1]->OnEvent(newEvent);
}

void BloomComponent::OnEvent(Event& event)
{
    if(event.GetType() == Event::Type::WindowResize)
    {
        const auto resizeEvent = dynamic_cast<WindowResizeEvent*>(&event);

        resolution = resizeEvent->GetSize();

        SetupPostProcessing();
    }
}

}

#include <PostProcessingComponents.hpp>

namespace lustra
{

GTAOComponent::GTAOComponent(const LLGL::Extent2D& resolution)
    : ComponentBase("GTAOComponent"), resolution(resolution)
{
    EventManager::Get().AddListener(Event::Type::WindowResize, this);

    const LLGL::Extent2D scaledResolution =
    {
        static_cast<uint32_t>(static_cast<float>(resolution.width) / resolutionScale),
        static_cast<uint32_t>(static_cast<float>(resolution.height) / resolutionScale)
    };

    gtao = std::make_shared<PostProcessing>(
        LLGL::PipelineLayoutDescriptor
        {
            .bindings =
            {
                { "gDepth", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 1 }
            },
            .uniforms =
            {
                { "far", LLGL::UniformType::Float1 },
                { "near", LLGL::UniformType::Float1 },
                { "samples", LLGL::UniformType::Int1 },
                { "limit", LLGL::UniformType::Float1 },
                { "radius", LLGL::UniformType::Float1 },
                { "falloff", LLGL::UniformType::Float1 },
                { "thicknessMix", LLGL::UniformType::Float1 },
                { "maxStride", LLGL::UniformType::Float1 }
            }
        },
        AssetManager::Get().Load<VertexShaderAsset>("screenRect.vert", true),
        AssetManager::Get().Load<FragmentShaderAsset>("GTAO.frag", true),
        scaledResolution,
        true,
        false,
        false,
        LLGL::Format::R16Float
    );

    boxBlur = std::make_shared<PostProcessing>(
        LLGL::PipelineLayoutDescriptor
        {
            .bindings =
            {
                { "frame", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 1 }
            }
        },
        AssetManager::Get().Load<VertexShaderAsset>("screenRect.vert", true),
        AssetManager::Get().Load<FragmentShaderAsset>("boxBlur.frag", true),
        scaledResolution,
        true,
        false,
        false,
        LLGL::Format::R16Float
    );
}

GTAOComponent::GTAOComponent(GTAOComponent&& other) noexcept
    : ComponentBase("GTAOComponent"),
      resolutionScale(other.resolutionScale), samples(other.samples), limit(other.limit),
      radius(other.radius), falloff(other.falloff), thicknessMix(other.thicknessMix), maxStride(other.maxStride),
      resolution(other.resolution), gtao(std::move(other.gtao)), boxBlur(std::move(other.boxBlur))
{
    EventManager::Get().AddListener(Event::Type::WindowResize, this);
}

GTAOComponent::~GTAOComponent()
{
    EventManager::Get().RemoveListener(Event::Type::WindowResize, this);
}

void GTAOComponent::SetupPostProcessing() const
{
    const LLGL::Extent2D scaledResolution =
    {
        static_cast<uint32_t>(static_cast<float>(resolution.width) / resolutionScale),
        static_cast<uint32_t>(static_cast<float>(resolution.height) / resolutionScale)
    };

    WindowResizeEvent newEvent(scaledResolution);

    gtao->OnEvent(newEvent);
    boxBlur->OnEvent(newEvent);
}

void GTAOComponent::OnEvent(Event& event)
{
    if(event.GetType() == Event::Type::WindowResize)
    {
        const auto resizeEvent = dynamic_cast<WindowResizeEvent*>(&event);

        resolution = resizeEvent->GetSize();

        SetupPostProcessing();
    }
}

}

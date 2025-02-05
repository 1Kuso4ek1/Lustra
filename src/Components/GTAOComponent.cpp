#include <PostProcessingComponents.hpp>

namespace dev
{

GTAOComponent::GTAOComponent(const LLGL::Extent2D& resolution)
    : ComponentBase("GTAOComponent"), resolution(resolution)
{
    EventManager::Get().AddListener(Event::Type::WindowResize, this);

    LLGL::Extent2D scaledResolution = 
    {
        (uint32_t)(resolution.width / resolutionScale),
        (uint32_t)(resolution.height / resolutionScale)
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
        LLGL::GraphicsPipelineDescriptor
        {
            .vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/screenRect.vert"),
            .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/GTAO.frag")
        },
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
        LLGL::GraphicsPipelineDescriptor
        {
            .vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/screenRect.vert"),
            .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/boxBlur.frag")
        },
        scaledResolution,
        true,
        false,
        false,
        LLGL::Format::R16Float
    );
}

GTAOComponent::~GTAOComponent()
{
    EventManager::Get().RemoveListener(Event::Type::WindowResize, this);
}

void GTAOComponent::SetupPostProcessing()
{
    LLGL::Extent2D scaledResolution = 
    {
        (uint32_t)(resolution.width / resolutionScale),
        (uint32_t)(resolution.height / resolutionScale)
    };

    WindowResizeEvent newEvent(scaledResolution);

    gtao->OnEvent(newEvent);
    boxBlur->OnEvent(newEvent);
}

void GTAOComponent::OnEvent(Event& event)
{
    if(event.GetType() == Event::Type::WindowResize)
    {
        auto resizeEvent = dynamic_cast<WindowResizeEvent*>(&event);

        resolution = resizeEvent->GetSize();

        SetupPostProcessing();
    }
}

}

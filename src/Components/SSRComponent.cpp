#include <PostProcessingComponents.hpp>

namespace dev
{

SSRComponent::SSRComponent(const LLGL::Extent2D& resolution)
    : ComponentBase("SSRComponent"), resolution(resolution)
{
    EventManager::Get().AddListener(Event::Type::WindowResize, this);

    LLGL::Extent2D scaledResolution = 
    {
        (uint32_t)(resolution.width / resolutionScale),
        (uint32_t)(resolution.height / resolutionScale)
    };

    ssr = std::make_shared<PostProcessing>(
        LLGL::PipelineLayoutDescriptor
        {
            .bindings =
            {
                { "matrices", LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::FragmentStage, 1 },
                { "gNormal", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 2 },
                { "gCombined", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 3 },
                { "depth", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 4 },
                { "frame", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 5 }
            },
            .uniforms =
            {
                { "maxSteps", LLGL::UniformType::Int1 },
                { "maxBinarySearchSteps", LLGL::UniformType::Int1 },
                { "rayStep", LLGL::UniformType::Float1 }
            }
        },
        LLGL::GraphicsPipelineDescriptor
        {
            .vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/screenRect.vert"),
            .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/SSR.frag")
        },
        scaledResolution,
        true,
        false,
        true
    );
}

SSRComponent::~SSRComponent()
{
    EventManager::Get().RemoveListener(Event::Type::WindowResize, this);
}

void SSRComponent::SetupPostProcessing()
{
    LLGL::Extent2D scaledResolution = 
    {
        (uint32_t)(resolution.width / resolutionScale),
        (uint32_t)(resolution.height / resolutionScale)
    };

    WindowResizeEvent newEvent(scaledResolution);

    ssr->OnEvent(newEvent);
}

void SSRComponent::OnEvent(Event& event)
{
    if(event.GetType() == Event::Type::WindowResize)
    {
        auto resizeEvent = dynamic_cast<WindowResizeEvent*>(&event);

        resolution = resizeEvent->GetSize();

        SetupPostProcessing();
    }
}

}

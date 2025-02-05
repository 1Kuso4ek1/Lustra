#include <PostProcessingComponents.hpp>

namespace dev
{

BloomComponent::BloomComponent(const LLGL::Extent2D& resolution)
    : ComponentBase("BloomComponent"), resolution(resolution)
{
    EventManager::Get().AddListener(Event::Type::WindowResize, this);

    LLGL::Extent2D scaledResolution =
    {
        (uint32_t)(resolution.width / resolutionScale),
        (uint32_t)(resolution.height / resolutionScale)
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

    LLGL::GraphicsPipelineDescriptor pingPongGraphics = 
    {
        .vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/screenRect.vert"),
        .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/blur.frag")
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
        LLGL::GraphicsPipelineDescriptor
        {
            .vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/screenRect.vert"),
            .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/threshold.frag")
        },
        scaledResolution,
        true,
        false
    );

    pingPong[0] = std::make_shared<PostProcessing>(
        pingPongLayout,
        pingPongGraphics,
        scaledResolution,
        true,
        false
    );

    pingPong[1] = std::make_shared<PostProcessing>(
        pingPongLayout,
        pingPongGraphics,
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

BloomComponent::~BloomComponent()
{
    EventManager::Get().RemoveListener(Event::Type::WindowResize, this);
}

void BloomComponent::SetupPostProcessing()
{
    LLGL::Extent2D scaledResolution = 
    {
        (uint32_t)(resolution.width / resolutionScale),
        (uint32_t)(resolution.height / resolutionScale)
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
        auto resizeEvent = dynamic_cast<WindowResizeEvent*>(&event);

        resolution = resizeEvent->GetSize();

        SetupPostProcessing();
    }
}

}

#include <Components.hpp>

#include <glm/gtx/matrix_decompose.hpp>

#include <chrono>

// Move everything into separate files...

namespace dev
{

void TransformComponent::SetTransform(const glm::mat4& transform)
{
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 position;
    glm::vec3 skew;
    glm::vec4 perspective;
    
    glm::decompose(transform, scale, rotation, position, skew, perspective);
    
    this->position = position;
    this->rotation = glm::degrees(glm::eulerAngles(rotation));
    this->scale = scale;
}

glm::mat4 TransformComponent::GetTransform() const
{
    return glm::translate(glm::mat4(1.0f), position)
            * glm::toMat4(glm::quat(glm::radians(rotation)))
            * glm::scale(glm::mat4(1.0f), scale);
}

LightComponent::LightComponent()
    : ComponentBase("LightComponent")
{
    projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);

    resolution = { 2048, 2048 };
}

void LightComponent::SetupShadowMap(const LLGL::Extent2D& resolution)
{
    if(depth)
    {
        Renderer::Get().Release(depth);
        Renderer::Get().Release(renderTarget);
    }

    CreateDepth(resolution);
    CreateRenderTarget(resolution);

    if(!shadowMapPipeline)
        CreatePipeline();
}

void LightComponent::CreateDepth(const LLGL::Extent2D& resolution)
{
    LLGL::TextureDescriptor depthDesc =
    {
        .type = LLGL::TextureType::Texture2D,
        .bindFlags = LLGL::BindFlags::DepthStencilAttachment | LLGL::BindFlags::Sampled,
        .format = LLGL::Format::D32Float,
        .extent = { resolution.width, resolution.height, 1 },
        .mipLevels = 1,
        .samples = 1
    };

    depth = Renderer::Get().CreateTexture(depthDesc);
}

void LightComponent::CreateRenderTarget(const LLGL::Extent2D& resolution)
{
    renderTarget = Renderer::Get().CreateRenderTarget(resolution, {}, depth);
}

void LightComponent::CreatePipeline()
{
    shadowMapPipeline = Renderer::Get().CreatePipelineState(
        LLGL::PipelineLayoutDescriptor
        {
            .bindings =
            {
                { "matrices", LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage, 1 }
            }
        },
        LLGL::GraphicsPipelineDescriptor
        {
            .renderPass = renderTarget->GetRenderPass(),
            .vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/depth.vert"),
            .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/depth.frag"),
            .depth = LLGL::DepthDescriptor
            {
                .testEnabled = true,
                .writeEnabled = true
            },
            .rasterizer = LLGL::RasterizerDescriptor
            {
                .cullMode = LLGL::CullMode::Back,
                .depthBias =
                {
                    .constantFactor = 4.0f,
                    .slopeFactor = 1.5f
                },
                .frontCCW = true
            },
            .blend =
            {
                .targets =
                {
                    {
                        .colorMask = 0x0
                    }
                }
            }
        }
    );
}

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
                { "gCombined", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 5 }
            },
            .uniforms
            {
                { "algorithm", LLGL::UniformType::Int1 },
                { "exposure", LLGL::UniformType::Float1 },
                { "bloomStrength", LLGL::UniformType::Float1 },
                { "colorGrading", LLGL::UniformType::Float3 },
                { "colorGradingIntensity", LLGL::UniformType::Float1 },
                { "chromaticAberration", LLGL::UniformType::Float1 },
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
        commandBuffer->SetUniforms(5, &chromaticAberration, sizeof(chromaticAberration));
        commandBuffer->SetUniforms(6, &vignetteIntensity, sizeof(vignetteIntensity));
        commandBuffer->SetUniforms(7, &vignetteRoundness, sizeof(vignetteRoundness));
        commandBuffer->SetUniforms(8, &filmGrain, sizeof(filmGrain));
        commandBuffer->SetUniforms(9, &contrast, sizeof(contrast));
        commandBuffer->SetUniforms(10, &saturation, sizeof(saturation));
        commandBuffer->SetUniforms(11, &brightness, sizeof(brightness));
        commandBuffer->SetUniforms(12, &time, sizeof(time));
    };
}

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

ProceduralSkyComponent::ProceduralSkyComponent(const LLGL::Extent2D& resolution)
    : ComponentBase("ProceduralSkyComponent"), resolution(resolution)
{
    pipeline = Renderer::Get().CreatePipelineState(
        LLGL::PipelineLayoutDescriptor
        {
            .bindings =
            {
                { "matrices", LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage, 1 }
            },
            .uniforms =
            {
                { "time", LLGL::UniformType::Float1 },
                { "cirrus", LLGL::UniformType::Float1 },
                { "cumulus", LLGL::UniformType::Float1 },
                { "flip", LLGL::UniformType::Bool1 }
            }
        },
        LLGL::GraphicsPipelineDescriptor
        {
            .vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/skybox.vert"),
            .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/proceduralSky.frag"),
            .depth = LLGL::DepthDescriptor
            {
                .testEnabled = true,
                .writeEnabled = false,
                .compareOp = LLGL::CompareOp::LessEqual
            },
            .rasterizer = LLGL::RasterizerDescriptor
            {
                .cullMode = LLGL::CullMode::Disabled,
                .frontCCW = true
            }
        }
    );

    setUniforms = [&](auto commandBuffer)
    {
        commandBuffer->SetUniforms(0, &time, sizeof(time));
        commandBuffer->SetUniforms(1, &cirrus, sizeof(cirrus));
        commandBuffer->SetUniforms(2, &cumulus, sizeof(cumulus));
        commandBuffer->SetUniforms(3, &flip, sizeof(flip));
    };

    Build();
}

void ProceduralSkyComponent::Build()
{
    if(asset)
    {
        if(asset->loaded)
        {
            Renderer::Get().Release(asset->cubeMap);
            Renderer::Get().Release(asset->irradiance);
            Renderer::Get().Release(asset->prefiltered);

            DefaultTextures();
        }
    }

    flip = 1;

    asset = PBRManager::Get().Build(
        resolution,
        AssetManager::Get().Load<TextureAsset>("default", true),
        asset,
        pipeline,
        setUniforms
    );

    flip = 0;
}

void ProceduralSkyComponent::DefaultTextures()
{
    auto defaultTexture = AssetManager::Get().Load<TextureAsset>("default", true)->texture;

    asset->cubeMap = defaultTexture;
    asset->irradiance = defaultTexture;
    asset->prefiltered = defaultTexture;
    asset->brdf = defaultTexture;
}

HDRISkyComponent::HDRISkyComponent(dev::TextureAssetPtr hdri, const LLGL::Extent2D& resolution)
    : ComponentBase("HDRISkyComponent"), environmentMap(hdri), resolution(resolution)
{
    EventManager::Get().AddListener(Event::Type::AssetLoaded, this);

    SetupSkyPipeline();

    if(hdri->loaded)
        Build();
    else
    {
        asset = std::make_shared<EnvironmentAsset>();
        
        DefaultTextures();
    }
}

void HDRISkyComponent::OnEvent(Event& event)
{
    if(event.GetType() == Event::Type::AssetLoaded)
    {
        auto assetLoadedEvent = static_cast<AssetLoadedEvent&>(event);

        if(assetLoadedEvent.GetAsset() == environmentMap)
            Build();
    }
}

void HDRISkyComponent::Build()
{
    if(asset)
    {
        if(asset->loaded)
        {
            Renderer::Get().Release(asset->cubeMap);
            Renderer::Get().Release(asset->irradiance);
            Renderer::Get().Release(asset->prefiltered);

            DefaultTextures();
        }
    }

    asset = PBRManager::Get().Build(resolution, environmentMap, asset);
}

void HDRISkyComponent::SetupSkyPipeline()
{
    pipelineSky = Renderer::Get().CreatePipelineState(
        LLGL::PipelineLayoutDescriptor
        {
            .bindings =
            {
                { "matrices", LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage, 1 },
                { "skybox", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 2 }
            }
        },
        LLGL::GraphicsPipelineDescriptor
        {
            .vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/skybox.vert"),
            .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/HDRISky.frag"),
            .depth = LLGL::DepthDescriptor
            {
                .testEnabled = true,
                .writeEnabled = false,
                .compareOp = LLGL::CompareOp::LessEqual
            },
            .rasterizer = LLGL::RasterizerDescriptor
            {
                .cullMode = LLGL::CullMode::Disabled,
                .frontCCW = true
            }
        }
    );
}

void HDRISkyComponent::DefaultTextures()
{
    auto defaultTexture = AssetManager::Get().Load<TextureAsset>("default", true)->texture;

    asset->cubeMap = defaultTexture;
    asset->irradiance = defaultTexture;
    asset->prefiltered = defaultTexture;
    asset->brdf = defaultTexture;
}

}

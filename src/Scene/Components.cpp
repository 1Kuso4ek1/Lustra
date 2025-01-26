#include <Components.hpp>

#include <glm/gtx/matrix_decompose.hpp>

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
                .frontCCW = true,
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

ACESTonemappingComponent::ACESTonemappingComponent(
    const LLGL::Extent2D& resolution,
    bool registerEvent
) : ComponentBase("ACESTonemappingComponent")
{
    postProcessing = std::make_shared<PostProcessing>(
        LLGL::PipelineLayoutDescriptor
        {
            .bindings =
            {
                { "frame", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 1 }
            },
            .uniforms
            {
                { "exposure", LLGL::UniformType::Float1 }
            }
        },
        LLGL::GraphicsPipelineDescriptor
        {
            .vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/screenRect.vert"),
            .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/ACES.frag")
        },
        resolution,
        true,
        registerEvent
    );

    setUniforms = [&](auto commandBuffer)
    {
        commandBuffer->SetUniforms(0, &exposure, sizeof(exposure));
    };
}

ProceduralSkyComponent::ProceduralSkyComponent()
    : ComponentBase("ProceduralSkyComponent")
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
                { "cumulus", LLGL::UniformType::Float1 }
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
    };
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

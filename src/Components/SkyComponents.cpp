#include <SkyComponents.hpp>

namespace lustra
{

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
            .vertexShader = AssetManager::Get().Load<VertexShaderAsset>("skybox.vert", true)->shader,
            .fragmentShader = AssetManager::Get().Load<FragmentShaderAsset>("proceduralSky.frag", true)->shader,
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

    MakeSetUniforms();

    Build();
}

ProceduralSkyComponent::ProceduralSkyComponent(ProceduralSkyComponent&& other)
    : ComponentBase("ProceduralSkyComponent"),
      time(other.time), cirrus(other.cirrus), cumulus(other.cumulus), flip(other.flip),
      asset(std::move(other.asset)), resolution(other.resolution), pipeline(other.pipeline)
{
    MakeSetUniforms();
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

void ProceduralSkyComponent::MakeSetUniforms()
{
    setUniforms = [&](auto commandBuffer)
    {
        commandBuffer->SetUniforms(0, &time, sizeof(time));
        commandBuffer->SetUniforms(1, &cirrus, sizeof(cirrus));
        commandBuffer->SetUniforms(2, &cumulus, sizeof(cumulus));
        commandBuffer->SetUniforms(3, &flip, sizeof(flip));
    };
}

void ProceduralSkyComponent::DefaultTextures() const
{
    const auto defaultTexture = AssetManager::Get().Load<TextureAsset>("default", true)->texture;

    asset->cubeMap = defaultTexture;
    asset->irradiance = defaultTexture;
    asset->prefiltered = defaultTexture;
    asset->brdf = defaultTexture;
}

HDRISkyComponent::HDRISkyComponent(const TextureAssetPtr& hdri, const LLGL::Extent2D& resolution)
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

HDRISkyComponent::HDRISkyComponent(HDRISkyComponent&& other)
    : ComponentBase("HDRISkyComponent"),
      environmentMap(std::move(other.environmentMap)), asset(std::move(other.asset)),
      resolution(other.resolution), pipelineSky(other.pipelineSky)
{
    EventManager::Get().AddListener(Event::Type::AssetLoaded, this);
}

HDRISkyComponent::~HDRISkyComponent()
{
    EventManager::Get().RemoveListener(Event::Type::AssetLoaded, this);
}

void HDRISkyComponent::OnEvent(Event& event)
{
    if(event.GetType() == Event::Type::AssetLoaded)
    {
        const auto assetLoadedEvent = static_cast<AssetLoadedEvent&>(event);

        if(assetLoadedEvent.GetAsset() == environmentMap)
            Build();
    }
}

void HDRISkyComponent::Build()
{
    if(asset)
    {
        if(asset->loaded && asset->cubeMap)
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
            .vertexShader = AssetManager::Get().Load<VertexShaderAsset>("skybox.vert", true)->shader,
            .fragmentShader = AssetManager::Get().Load<FragmentShaderAsset>("HDRISky.frag", true)->shader,
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

void HDRISkyComponent::DefaultTextures() const
{
    const auto defaultTexture = AssetManager::Get().Load<TextureAsset>("default", true)->texture;

    asset->cubeMap = defaultTexture;
    asset->irradiance = defaultTexture;
    asset->prefiltered = defaultTexture;
    asset->brdf = defaultTexture;
}

}

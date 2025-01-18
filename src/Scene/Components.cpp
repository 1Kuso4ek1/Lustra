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
        .bindFlags = LLGL::BindFlags::DepthStencilAttachment,
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
            .vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/depth.vert"),
            .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/depth.frag"),
            .depth = LLGL::DepthDescriptor
            {
                .testEnabled = true,
                .writeEnabled = true,
                //.compareOp = LLGL::CompareOp::LessEqual
            },
            .rasterizer = LLGL::RasterizerDescriptor
            {
                .cullMode = LLGL::CullMode::Back,
                .frontCCW = true
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
                { "frame", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 1 },
                { "samplerState", LLGL::ResourceType::Sampler, 0, LLGL::StageFlags::FragmentStage, 1 }
            },
            .uniforms
            {
                { "exposure", LLGL::UniformType::Float1 }
            }
        },
        LLGL::GraphicsPipelineDescriptor
        {
            .vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/screenRect.vert"),
            .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/ACES.frag"),
            .primitiveTopology = LLGL::PrimitiveTopology::TriangleList
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

    SetupConvertPipeline();
    SetupSkyPipeline();

    CreateCubemap(resolution);
    CreateRenderTargets(resolution);

    if(hdri->loaded)
        Convert();
}

void HDRISkyComponent::OnEvent(Event& event)
{
    if(event.GetType() == Event::Type::AssetLoaded)
    {
        auto assetLoadedEvent = static_cast<AssetLoadedEvent&>(event);

        if(assetLoadedEvent.GetAsset() == environmentMap)
            Convert();
    }
}

void HDRISkyComponent::SetResolution(const LLGL::Extent2D& resolution)
{
    if(cubeMap)
        Renderer::Get().Release(cubeMap);

    for(auto renderTarget : renderTargets)
        if(renderTarget)
            Renderer::Get().Release(renderTarget);

    CreateCubemap(resolution);
    CreateRenderTargets(resolution);

    if(environmentMap->loaded)
        dev::Multithreading::Get().AddMainThreadJob([&]() { Convert(); });
}

void HDRISkyComponent::SetupConvertPipeline()
{
    pipelineConvert = Renderer::Get().CreatePipelineState(
        LLGL::PipelineLayoutDescriptor
        {
            .bindings =
            {
                { "matrices", LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage, 1 },
                { "hdri", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 2 },
                { "samplerState", LLGL::ResourceType::Sampler, 0, LLGL::StageFlags::FragmentStage, 2 }
            }
        },
        LLGL::GraphicsPipelineDescriptor
        {
            .vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/skybox.vert"),
            .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/HDRIConvert.frag"),
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

void HDRISkyComponent::SetupSkyPipeline()
{
    pipelineSky = Renderer::Get().CreatePipelineState(
        LLGL::PipelineLayoutDescriptor
        {
            .bindings =
            {
                { "matrices", LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage, 1 },
                { "skybox", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 2 },
                { "samplerState", LLGL::ResourceType::Sampler, 0, LLGL::StageFlags::FragmentStage, 2 }
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

void HDRISkyComponent::CreateCubemap(const LLGL::Extent2D& resolution)
{
    cubeMap = Renderer::Get().CreateTexture(
        LLGL::TextureDescriptor
        {
            .type = LLGL::TextureType::TextureCubeArray,
            .bindFlags = LLGL::BindFlags::ColorAttachment | LLGL::BindFlags::Sampled,
            .format = LLGL::Format::RGBA16Float,
            .extent = { resolution.width, resolution.height, 1 },
            .arrayLayers = 6
        }
    );

    // LLGL::NumMipLevels(resolution.width, resolution.height);
}

void HDRISkyComponent::CreateRenderTargets(const LLGL::Extent2D& resolution)
{
    for(int i = 0; i < 6; i++)
    {
        renderTargets[i] = Renderer::Get().CreateRenderTarget(
            { resolution.width, resolution.height },
            { LLGL::AttachmentDescriptor(cubeMap, 0, i) }
        );
    }
}

void HDRISkyComponent::Convert()
{    
    auto matrices = Renderer::Get().GetMatrices();

    matrices->PushMatrix();

    matrices->GetModel() = glm::mat4(1.0f);
    matrices->GetProjection() = projection;

    auto cube = AssetManager::Get().Load<ModelAsset>("cube", true);
    
    for(int i = 0; i < 6; i++)
    {
        matrices->GetView() = views[i];

        Renderer::Get().Begin();

        Renderer::Get().RenderPass(
            [&](auto commandBuffer)
            {
                cube->meshes[0]->BindBuffers(commandBuffer);
            },
            {
                { 0, Renderer::Get().GetMatricesBuffer() },
                { 1, environmentMap->texture },
                { 2, environmentMap->sampler }
            },
            [&](auto commandBuffer)
            {
                cube->meshes[0]->Draw(commandBuffer);
            },
            pipelineConvert,
            renderTargets[i]
        );

        Renderer::Get().End();
        
        Renderer::Get().Submit();
    }

    Renderer::Get().GenerateMips(cubeMap);

    matrices->PopMatrix();
}

}

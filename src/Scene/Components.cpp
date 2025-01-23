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
    SetupIrradiancePipeline();
    SetupPrefilteredPipeline();
    SetupBRDFPipeline();
    SetupSkyPipeline();

    CreateCubemaps(resolution);
    CreateRenderTargets(resolution, cubeMap);
    
    CreateBRDFTexture({ 512, 512 });
    CreateBRDFRenderTarget({ 512, 512 });

    RenderBRDF();

    if(hdri->loaded)
        Build();
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

void HDRISkyComponent::SetResolution(const LLGL::Extent2D& resolution)
{
    ReleaseCubeMaps();
    ReleaseRenderTargets();

    CreateCubemaps(resolution);
    
    if(environmentMap->loaded)
        dev::Multithreading::Get().AddMainThreadJob([&]() { Build(); });
}

void HDRISkyComponent::Build()
{
    CreateRenderTargets(resolution, cubeMap);

    RenderCubeMap(
        {
            { 1, environmentMap->texture },
            { 2, environmentMap->sampler }
        },
        cubeMap,
        pipelineConvert
    );

    ReleaseRenderTargets();
    CreateRenderTargets({ 32, 32 }, irradiance);

    RenderCubeMap(
        {
            { 1, cubeMap },
            { 2, environmentMap->sampler }
        },
        irradiance,
        pipelineIrradiance
    );

    ReleaseRenderTargets();
    CreateRenderTargets({ 1024, 1024 }, prefiltered);

    RenderCubeMapMips(
        {
            { 1, cubeMap },
            { 2, environmentMap->sampler }
        },
        prefiltered,
        pipelinePrefiltered
    );
}

void HDRISkyComponent::RenderCubeMap(
    const std::unordered_map<uint32_t, LLGL::Resource*>& resources,
    LLGL::Texture* cubeMap,
    LLGL::PipelineState* pipeline
)
{    
    auto matrices = Renderer::Get().GetMatrices();

    matrices->PushMatrix();

    matrices->GetModel() = glm::mat4(1.0f);
    matrices->GetProjection() = projection;

    auto cube = AssetManager::Get().Load<ModelAsset>("cube", true)->meshes[0];
    
    for(int i = 0; i < 6; i++)
    {
        matrices->GetView() = views[i];

        Renderer::Get().Begin();

        Renderer::Get().RenderPass(
            [&](auto commandBuffer)
            {
                cube->BindBuffers(commandBuffer);
            },
            {
                { 0, Renderer::Get().GetMatricesBuffer() },
                { 1, resources.at(1) },
                { 2, resources.at(2) }
            },
            [&](auto commandBuffer)
            {
                cube->Draw(commandBuffer);
            },
            pipeline,
            renderTargets[i]
        );

        Renderer::Get().End();
        
        Renderer::Get().Submit();
    }

    Renderer::Get().GenerateMips(cubeMap);

    matrices->PopMatrix();
}

void HDRISkyComponent::RenderCubeMapMips(
    const std::unordered_map<uint32_t, LLGL::Resource*>& resources,
    LLGL::Texture* cubeMap,
    LLGL::PipelineState* pipeline
)
{
    auto matrices = Renderer::Get().GetMatrices();

    matrices->PushMatrix();

    matrices->GetModel() = glm::mat4(1.0f);
    matrices->GetProjection() = projection;

    auto cube = AssetManager::Get().Load<ModelAsset>("cube", true)->meshes[0];

    auto initialResolution = renderTargets[0]->GetResolution();

    auto mipsNum = 
        LLGL::NumMipLevels(
            initialResolution.width,
            initialResolution.height
        );

    for(int i = 0; i < mipsNum; i++)
    {
        for(int j = 0; j < 6; j++)
        {
            matrices->GetView() = views[j];

            Renderer::Get().Begin();

            Renderer::Get().RenderPass(
                [&](auto commandBuffer)
                {
                    cube->BindBuffers(commandBuffer);
                },
                {
                    { 0, Renderer::Get().GetMatricesBuffer() },
                    { 1, resources.at(1) },
                    { 2, resources.at(2) }
                },
                [&](auto commandBuffer)
                {
                    float roughness = (float)i / (float)(mipsNum - 1);

                    commandBuffer->SetUniforms(0, &roughness, sizeof(roughness));

                    cube->Draw(commandBuffer);
                },
                pipeline,
                renderTargets[j]
            );

            Renderer::Get().End();

            Renderer::Get().Submit();
        }

        ReleaseRenderTargets();
        CreateRenderTargets(
            {
                initialResolution.width / (int)std::pow(2, i),
                initialResolution.height / (int)std::pow(2, i)
            },
            prefiltered,
            i
        );
    }

    matrices->PopMatrix();
}

void HDRISkyComponent::RenderBRDF()
{
    auto plane = AssetManager::Get().Load<ModelAsset>("plane", true)->meshes[0];

    Renderer::Get().Begin();

    Renderer::Get().RenderPass(
        [&](auto commandBuffer)
        {
            plane->BindBuffers(commandBuffer, false);
        },
        {},
        [&](auto commandBuffer)
        {
            plane->Draw(commandBuffer);
        },
        pipelineBRDF,
        brdfRenderTarget
    );

    Renderer::Get().End();

    Renderer::Get().Submit();
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
                .cullMode = LLGL::CullMode::Front,
                .frontCCW = true
            }
        }
    );
}

void HDRISkyComponent::SetupIrradiancePipeline()
{
    pipelineIrradiance = Renderer::Get().CreatePipelineState(
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
            .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/irradiance.frag"),
            .depth = LLGL::DepthDescriptor
            {
                .testEnabled = true,
                .writeEnabled = false,
                .compareOp = LLGL::CompareOp::LessEqual
            },
            .rasterizer = LLGL::RasterizerDescriptor
            {
                .cullMode = LLGL::CullMode::Front,
                .frontCCW = true
            }
        }
    );
}

void HDRISkyComponent::SetupPrefilteredPipeline()
{
    pipelinePrefiltered = Renderer::Get().CreatePipelineState(
        LLGL::PipelineLayoutDescriptor
        {
            .bindings =
            {
                { "matrices", LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage, 1 },
                { "skybox", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, 2 },
                { "samplerState", LLGL::ResourceType::Sampler, 0, LLGL::StageFlags::FragmentStage, 2 }
            },
            .uniforms = 
            {
                { "roughness", LLGL::UniformType::Float1 }
            }
        },
        LLGL::GraphicsPipelineDescriptor
        {
            .vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/skybox.vert"),
            .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/prefilter.frag"),
            .depth = LLGL::DepthDescriptor
            {
                .testEnabled = true,
                .writeEnabled = false,
                .compareOp = LLGL::CompareOp::LessEqual
            },
            .rasterizer = LLGL::RasterizerDescriptor
            {
                .cullMode = LLGL::CullMode::Front,
                .frontCCW = true
            }
        }
    );
}

void HDRISkyComponent::SetupBRDFPipeline()
{
    pipelineBRDF = Renderer::Get().CreatePipelineState({},
        {
            .vertexShader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/screenRect.vert"),
            .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/BRDF.frag"),
            .primitiveTopology = LLGL::PrimitiveTopology::TriangleList
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

void HDRISkyComponent::CreateCubemaps(const LLGL::Extent2D& resolution)
{
    LLGL::TextureDescriptor textureDesc
    {
        .type = LLGL::TextureType::TextureCubeArray,
        .bindFlags = LLGL::BindFlags::ColorAttachment | LLGL::BindFlags::Sampled,
        .format = LLGL::Format::RGB16Float,
        .extent = { resolution.width, resolution.height, 1 },
        .arrayLayers = 6
    };

    cubeMap = Renderer::Get().CreateTexture(textureDesc);
    Renderer::Get().GenerateMips(cubeMap);

    textureDesc.extent = { 32, 32, 1 }; // resolution.width / 64
    irradiance = Renderer::Get().CreateTexture(textureDesc);

    textureDesc.extent = { 1024, 1024, 1 }; // resolution.width / 2
    prefiltered = Renderer::Get().CreateTexture(textureDesc);
    Renderer::Get().GenerateMips(prefiltered);
    

    // LLGL::NumMipLevels(resolution.width, resolution.height);
}

void HDRISkyComponent::CreateRenderTargets(const LLGL::Extent2D& resolution, LLGL::Texture* cubeMap, int mipLevel)
{
    for(int i = 0; i < 6; i++)
    {
        renderTargets[i] = Renderer::Get().CreateRenderTarget(
            resolution,
            { LLGL::AttachmentDescriptor(cubeMap, mipLevel, i) }
        );
    }
}

void HDRISkyComponent::CreateBRDFTexture(const LLGL::Extent2D& resolution)
{
    LLGL::TextureDescriptor textureDesc
    {
        .type = LLGL::TextureType::Texture2D,
        .bindFlags = LLGL::BindFlags::ColorAttachment | LLGL::BindFlags::Sampled,
        .format = LLGL::Format::RG16Float,
        .extent = { resolution.width, resolution.height, 1 },
        .mipLevels = 1
    };

    brdf = Renderer::Get().CreateTexture(textureDesc);
}

void HDRISkyComponent::CreateBRDFRenderTarget(const LLGL::Extent2D& resolution)
{
    brdfRenderTarget = Renderer::Get().CreateRenderTarget(resolution, { brdf });
}

void HDRISkyComponent::ReleaseCubeMaps()
{
    if(cubeMap)
    {
        Renderer::Get().Release(cubeMap);
        Renderer::Get().Release(irradiance);
        Renderer::Get().Release(prefiltered);
    }
}

void HDRISkyComponent::ReleaseRenderTargets()
{
    for(auto renderTarget : renderTargets)
        if(renderTarget)
            Renderer::Get().Release(renderTarget);
}


}

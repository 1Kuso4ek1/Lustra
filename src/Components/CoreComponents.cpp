#include <CoreComponents.hpp>

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
    SetupProjection();

    resolution = { 2048, 2048 };
}

void LightComponent::SetupProjection()
{
    if(orthographic)
        projection = glm::ortho(-orthoExtent, orthoExtent, -orthoExtent, orthoExtent, 0.1f, 1000.0f);
    else
        projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
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

    SetupProjection();

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
            .vertexShader = AssetManager::Get().Load<VertexShaderAsset>("depth.vert", true)->shader,
            .fragmentShader = AssetManager::Get().Load<FragmentShaderAsset>("depth.frag", true)->shader,
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

}

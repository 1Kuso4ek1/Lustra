#include <Scene.hpp>
#include <Entity.hpp>

namespace dev
{

Scene::Scene(std::shared_ptr<RendererBase> renderer)
    : renderer(renderer)
{
}

void Scene::SetRenderer(std::shared_ptr<RendererBase> renderer)
{
    this->renderer = renderer;
}

void Scene::Start()
{
    if(!lightsBuffer)
    {
        SetupLightsBuffer();
        SetupShadowsBuffer();
        // SetupCombinePostProcessing();
    }

    registry.view<ScriptComponent>().each([](auto& script)
    {
        if(script.start)
            script.start();
    });
}

void Scene::Update(float deltaTime)
{
    registry.view<ScriptComponent>().each([&](auto entity, auto& script)
    {
        if(script.update)
            script.update(Entity{ entity, this }, deltaTime);
    });
}

void Scene::Draw(LLGL::RenderTarget* renderTarget)
{
    RenderToShadowMap();

    SetupCamera();
    SetupLights();
    SetupShadows();

    Renderer::Get().Begin();

    UpdateLightsBuffer();
    UpdateShadowsBuffer();

    RenderMeshes();
    
    Renderer::Get().End();

    Renderer::Get().Submit();

    ApplyPostProcessing(renderTarget);
}

void Scene::RemoveEntity(const Entity& entity)
{
    registry.destroy(entity);
}

Entity Scene::CreateEntity()
{
    Entity entity{ registry.create(), this };

    entities[idCounter++] = entity;

    return entity;
}

entt::registry& Scene::GetRegistry()
{
    return registry;
}

void Scene::SetupLightsBuffer()
{
    static const uint64_t maxLights = 128;

    LLGL::BufferDescriptor lightBufferDesc = LLGL::ConstantBufferDesc(maxLights * sizeof(Light));
    
    lightsBuffer = Renderer::Get().CreateBuffer(lightBufferDesc);

    lights.reserve(maxLights);
}

void Scene::SetupShadowsBuffer()
{
    static const uint64_t maxShadows = 4;

    LLGL::BufferDescriptor shadowBufferDesc = LLGL::ConstantBufferDesc(maxShadows * sizeof(Shadow));

    shadowsBuffer = Renderer::Get().CreateBuffer(shadowBufferDesc);

    shadows.reserve(maxShadows);
}

void Scene::UpdateLightsBuffer()
{
    Renderer::Get().RenderPass(
        [&](auto commandBuffer)
        {
            commandBuffer->UpdateBuffer(*lightsBuffer, 0, lights.data(), lights.size() * sizeof(Light));
        }, {}, [](auto) {}, nullptr
    );
}

void Scene::UpdateShadowsBuffer()
{
    Renderer::Get().RenderPass(
        [&](auto commandBuffer)
        {
            commandBuffer->UpdateBuffer(*shadowsBuffer, 0, shadows.data(), shadows.size() * sizeof(Shadow));
        }, {}, [](auto) {}, nullptr
    );
}

void Scene::SetupCamera()
{
    auto cameraView = registry.view<TransformComponent, CameraComponent>();

    Camera* cam{};
    TransformComponent cameraTransform;

    for(auto entity : cameraView)
    {
        cam = &cameraView.get<CameraComponent>(entity).camera;
        cameraTransform = cameraView.get<TransformComponent>(entity);

        cameraPosition = cameraTransform.position;

        break;
    }

    if(cam)
    {
        auto delta = glm::quat(glm::radians(cameraTransform.rotation)) * glm::vec3(0.0f, 0.0f, -1.0f);
        
        Renderer::Get().GetMatrices()->GetView() = glm::lookAt(cameraTransform.position, cameraTransform.position + delta, glm::vec3(0.0f, 1.0f, 0.0f));
        Renderer::Get().GetMatrices()->GetProjection() = cam->GetProjectionMatrix();
    }
}

void Scene::SetupLights()
{
    lights.clear();

    auto lightsView = registry.view<LightComponent, TransformComponent>();

    for(auto entity : lightsView)
    {
        auto [light, transform] = lightsView.get<LightComponent, TransformComponent>(entity);

        lights.push_back(
            {
                transform.position,
                glm::quat(glm::radians(transform.rotation)) * glm::vec3(0.0f, 0.0f, -1.0f),
                light.color,
                light.intensity,
                glm::cos(glm::radians(light.cutoff)),
                glm::cos(glm::radians(light.outerCutoff))
            }
        );
    }
}

void Scene::SetupShadows()
{
    shadows.clear();

    // Fill shadowSamplers with default texture
    std::fill(
        shadowSamplers.begin(),
        shadowSamplers.end(),
        AssetManager::Get().Load<TextureAsset>("default", true)->texture
    );

    auto lightsView = registry.view<LightComponent, TransformComponent>();

    for(auto entity : lightsView)
    {
        auto [light, transform] =
            lightsView.get<LightComponent, TransformComponent>(entity);

        if(light.shadowMap)
        {
            auto delta = glm::quat(glm::radians(transform.rotation)) * glm::vec3(0.0f, 0.0f, -1.0f);
        
            shadows.push_back(
                {
                    light.projection *
                    glm::lookAt(transform.position, transform.position + delta, glm::vec3(0.0f, 1.0f, 0.0f)),
                    light.bias
                }
            );

            shadowSamplers[shadows.size() - 1] = light.depth;
        }
    }
}

// ???
void Scene::SetupCombinePostProcessing()
{
    combinePost = std::make_shared<PostProcessing>(
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
            .fragmentShader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/combinePost.frag")
        },
        Renderer::Get().GetSwapChain()->GetResolution(),
        true,
        true
    );
}

void Scene::RenderMeshes()
{
    auto view = registry.view<TransformComponent, MeshComponent, MeshRendererComponent, PipelineComponent>();
    
    for(auto entity : view)
    {
        auto [transform, mesh, meshRenderer, pipeline] = 
                view.get<TransformComponent, MeshComponent, MeshRendererComponent, PipelineComponent>(entity);

        Renderer::Get().GetMatrices()->PushMatrix();
        Renderer::Get().GetMatrices()->GetModel() = transform.GetTransform();

        MeshRenderPass(mesh, meshRenderer, pipeline, renderer->GetPrimaryRenderTarget());

        Renderer::Get().GetMatrices()->PopMatrix();
    }

    if(view.begin() == view.end())
        dev::Renderer::Get().ClearRenderTarget(renderer->GetPrimaryRenderTarget());
}

void Scene::RenderToShadowMap()
{
    Renderer::Get().Begin();

    auto meshesView = registry.view<TransformComponent, MeshComponent, MeshRendererComponent, PipelineComponent>();
    auto lightsView = registry.view<LightComponent, TransformComponent>();

    for(auto light : lightsView)
    {
        auto [lightComponent, lightTransform] = 
                lightsView.get<LightComponent, TransformComponent>(light);

        if(lightComponent.shadowMap && lightComponent.renderTarget)
        {
            auto delta = glm::quat(glm::radians(lightTransform.rotation)) * glm::vec3(0.0f, 0.0f, -1.0f);

            Renderer::Get().GetMatrices()->GetView() = glm::lookAt(lightTransform.position, lightTransform.position + delta, glm::vec3(0.0f, 1.0f, 0.0f));
            Renderer::Get().GetMatrices()->GetProjection() = lightComponent.projection;

            for(auto mesh : meshesView)
            {
                auto [transform, meshComp, meshRenderer, pipeline] =
                        meshesView.get<TransformComponent, MeshComponent, MeshRendererComponent, PipelineComponent>(mesh);

                Renderer::Get().GetMatrices()->PushMatrix();
                Renderer::Get().GetMatrices()->GetModel() = transform.GetTransform();

                ShadowRenderPass(lightComponent, meshComp);

                Renderer::Get().GetMatrices()->PopMatrix();
            }
        }
    }

    Renderer::Get().End();

    Renderer::Get().Submit();
}

void Scene::RenderSky(LLGL::RenderTarget* renderTarget)
{
    auto hdriSkyView = registry.view<MeshComponent, HDRISkyComponent>();

    if(hdriSkyView.begin() != hdriSkyView.end())
    {
        auto [mesh, sky] = 
            hdriSkyView.get<MeshComponent, HDRISkyComponent>(*hdriSkyView.begin());

        HDRISkyRenderPass(mesh, sky, renderTarget);

        return;
    }

    auto proceduralSkyView = registry.view<MeshComponent, ProceduralSkyComponent>();

    if(proceduralSkyView.begin() != proceduralSkyView.end())
    {
        auto [mesh, sky] = proceduralSkyView.get<MeshComponent, ProceduralSkyComponent>(*proceduralSkyView.begin());

        ProceduralSkyRenderPass(mesh, sky, renderTarget);
    }
}

void Scene::MeshRenderPass(MeshComponent mesh, MeshRendererComponent meshRenderer, PipelineComponent pipeline, LLGL::RenderTarget* renderTarget)
{
    for(size_t i = 0; i < mesh.model->meshes.size(); i++)
    {
        auto material = AssetManager::Get().Load<MaterialAsset>("default", true);

        if(meshRenderer.materials.size() > i)
            material = meshRenderer.materials[i];

        Renderer::Get().RenderPass(
            [&](auto commandBuffer)
            {
                mesh.model->meshes[i]->BindBuffers(commandBuffer);
            },
            {
                { 0, Renderer::Get().GetMatricesBuffer() },
                { 1, material->albedo.texture->texture },
                { 2, material->normal.texture->texture },
                { 3, material->metallic.texture->texture },
                { 4, material->roughness.texture->texture },
                { 5, material->ao.texture->texture },
                { 6, material->emission.texture->texture },
                { 7, material->albedo.texture->sampler }
            },
            [&](auto commandBuffer)
            {
                material->SetUniforms(commandBuffer);

                mesh.model->meshes[i]->Draw(commandBuffer);
            },
            pipeline.pipeline,
            renderer->GetPrimaryRenderTarget()
        );
    }
}

void Scene::ShadowRenderPass(LightComponent light, MeshComponent mesh)
{
    for(size_t i = 0; i < mesh.model->meshes.size(); i++)
    {
        Renderer::Get().RenderPass(
            [&](auto commandBuffer)
            {
                mesh.model->meshes[i]->BindBuffers(commandBuffer);
            },
            { { 0, Renderer::Get().GetMatricesBuffer() } },
            [&](auto commandBuffer)
            {
                mesh.model->meshes[i]->Draw(commandBuffer);
            },
            light.shadowMapPipeline,
            light.renderTarget
        );
    }
}

void Scene::ProceduralSkyRenderPass(MeshComponent mesh, ProceduralSkyComponent sky, LLGL::RenderTarget* renderTarget)
{
    Renderer::Get().RenderPass(
        [&](auto commandBuffer)
        {
            mesh.model->meshes[0]->BindBuffers(commandBuffer);
        },
        {
            { 0, Renderer::Get().GetMatricesBuffer() }
        },
        [&](auto commandBuffer)
        {
            sky.setUniforms(commandBuffer);
            
            mesh.model->meshes[0]->Draw(commandBuffer);
        },
        sky.pipeline,
        renderTarget
    );
}

void Scene::HDRISkyRenderPass(MeshComponent mesh, HDRISkyComponent sky, LLGL::RenderTarget* renderTarget)
{
    Renderer::Get().RenderPass(
        [&](auto commandBuffer)
        {
            mesh.model->meshes[0]->BindBuffers(commandBuffer);
        },
        {
            { 0, Renderer::Get().GetMatricesBuffer() },
            { 1, sky.asset->cubeMap },
            { 2, sky.environmentMap->sampler }
        },
        [&](auto commandBuffer)
        {
            mesh.model->meshes[0]->Draw(commandBuffer);
        },
        sky.pipelineSky,
        renderTarget
    );
}

void Scene::RenderResult(LLGL::RenderTarget* renderTarget)
{
    static auto uniforms = [&](auto commandBuffer)
    {
        auto numLights = lights.size();
        auto numShadows = shadows.size();

        commandBuffer->SetUniforms(0, &numLights, sizeof(numLights));
        commandBuffer->SetUniforms(1, &numShadows, sizeof(numShadows));
        commandBuffer->SetUniforms(2, &cameraPosition, sizeof(cameraPosition));
    };

    Renderer::Get().Begin();

    RenderSky(renderTarget);

    auto defaultTexture = AssetManager::Get().Load<TextureAsset>("default", true)->texture;
    auto irradiance = defaultTexture;
    auto prefiltered = defaultTexture;
    auto brdf = defaultTexture;

    // Very bad, obviously
    auto hdriSkyView = registry.view<MeshComponent, HDRISkyComponent>();

    if(hdriSkyView.begin() != hdriSkyView.end())
    {
        auto sky = hdriSkyView.get<HDRISkyComponent>(*hdriSkyView.begin());

        irradiance = sky.asset->irradiance;
        prefiltered = sky.asset->prefiltered;
        brdf = sky.asset->brdf;
    }

    renderer->Draw(
        {
            { 5, lightsBuffer },
            { 6, shadowsBuffer },

            { 7, shadowSamplers[0] },
            { 8, shadowSamplers[1] },
            { 9, shadowSamplers[2] },
            { 10, shadowSamplers[3] },

            { 11, irradiance },
            { 12, prefiltered },
            { 13, brdf }
        },
        uniforms,
        renderTarget
    );

    Renderer::Get().End();

    Renderer::Get().Submit();
}

void Scene::ApplyPostProcessing(LLGL::RenderTarget* renderTarget)
{
    auto acesView = registry.view<ACESTonemappingComponent>();

    if(acesView->begin() == acesView->end())
    {
        RenderResult(renderTarget);
        return;
    }

    auto postProcessing = *acesView->begin();

    RenderResult(postProcessing.postProcessing->GetRenderTarget());

    postProcessing.postProcessing->Apply(
        {
            { 0, postProcessing.postProcessing->GetFrame() }
        },
        postProcessing.setUniforms,
        renderTarget
    );
}
    
}

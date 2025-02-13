#include <Scene.hpp>
#include <Entity.hpp>
#include <ScriptManager.hpp>

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
    }

    registry.view<ScriptComponent>().each([&](auto entity, auto& script)
    {
        if(script.script)
        {
            Entity ent{ entity, this };

            ScriptManager::Get().ExecuteFunction(
                script.script,
                "void Start(Scene@, Entity)",
                [&](auto context)
                {
                    context->SetArgAddress(0, (void*)(this));
                    context->SetArgObject(1, (void*)(&ent));
                },
                script.moduleIndex
            );
        }

        if(script.start)
            script.start();
    });
}

void Scene::Update(float deltaTime)
{
    InputManager::Get().Update();

    registry.view<ScriptComponent>().each([&](auto entity, auto& script)
    {
        if(script.script)
        {
            Entity ent{ entity, this };

            ScriptManager::Get().ExecuteFunction(
                script.script,
                "void Update(Scene@, Entity, float)",
                [&](auto context)
                {
                    context->SetArgAddress(0, (void*)(this));
                    context->SetArgObject(1, (void*)(&ent));
                    context->SetArgFloat(2, deltaTime);
                },
                script.moduleIndex
            );
        }

        if(script.update)
            script.update(Entity{ entity, this }, deltaTime);
    });

    if(updatePhysics)
        PhysicsManager::Get().Update(deltaTime);
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

void Scene::SetUpdatePhysics(bool updatePhysics)
{
    this->updatePhysics = updatePhysics;
}

void Scene::ToggleUpdatePhysics()
{
    updatePhysics = !updatePhysics;
}

void Scene::ReparentEntity(Entity child, Entity parent)
{
    static auto removeChild = [&](Entity child, Entity parent)
    {
        if(registry.valid(parent))
        {
            auto& prevParentHierarchy = parent.GetComponent<dev::HierarchyComponent>();

            prevParentHierarchy.children.erase(
                std::remove(prevParentHierarchy.children.begin(), 
                            prevParentHierarchy.children.end(), child),
                prevParentHierarchy.children.end()
            );
        }
    };

    auto& childHierarchy = child.GetOrAddComponent<dev::HierarchyComponent>();
    auto prevParent = Entity(childHierarchy.parent, this);

    if(prevParent == parent)
    {
        childHierarchy.parent = entt::null;

        removeChild(child, prevParent);

        return;
    }

    if(child == parent || IsChildOf(child, parent))
        return;

    removeChild(child, prevParent);

    childHierarchy.parent = parent;
    if(registry.valid(parent))
    {
        auto& parentHierarchy = parent.GetOrAddComponent<dev::HierarchyComponent>();
        parentHierarchy.children.push_back(child);

        if(child.HasComponent<dev::TransformComponent>())
        {
            auto& childTransform = child.GetComponent<dev::TransformComponent>();
            auto parentWorld = GetWorldTransform(parent);
            childTransform.SetTransform(glm::inverse(parentWorld) * childTransform.GetTransform());
        }
    }
}

void Scene::RemoveEntity(Entity entity)
{
    registry.destroy(entity);
}

Entity Scene::CreateEntity()
{
    return { registry.create(), this };
}

Entity Scene::GetEntity(entt::id_type id)
{
    return { entt::entity(id), this };
}

Entity Scene::GetEntity(const std::string& name)
{
    Entity ret{};

    registry.view<NameComponent>().each([&](auto entity, auto& component)
    {
        if(component.name == name)
            ret = { entity, this };
    });

    return ret;
}

bool Scene::IsChildOf(Entity child, Entity parent)
{
    if(!registry.valid(child) || !registry.valid(parent))
        return false;

    entt::entity current = child;

    while(registry.all_of<HierarchyComponent>(current))
    {
        auto& hierarchy = registry.get<HierarchyComponent>(current);
        current = hierarchy.parent;

        if(current == parent)
            return true;
    }

    return false;
}

glm::mat4 Scene::GetWorldTransform(entt::entity entity)
{
    entt::entity parent = entity;

    if(!registry.all_of<TransformComponent>(parent))
        return glm::mat4(1.0f);

    glm::mat4 transformMatrix = registry.get<TransformComponent>(entity).GetTransform();

    while(registry.all_of<HierarchyComponent>(parent))
    {
        auto& hierarchy = registry.get<HierarchyComponent>(parent);
        parent = hierarchy.parent;

        if(registry.valid(parent))
        {
            auto& parentTransform = registry.get<TransformComponent>(parent);
            transformMatrix = parentTransform.GetTransform() * transformMatrix;
        }
        else
            break;
    }

    return transformMatrix;
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

    TransformComponent cameraTransform;

    cam = nullptr;

    for(auto entity : cameraView)
    {
        cam = &cameraView.get<CameraComponent>(entity).camera;
        cameraTransform = cameraView.get<TransformComponent>(entity);

        if(registry.all_of<HierarchyComponent>(entity))
        {
            auto rotation = cameraTransform.rotation;

            cameraTransform.SetTransform(GetWorldTransform(entity));

            cameraTransform.rotation = rotation;
        }

        cameraPosition = cameraTransform.position;

        break;
    }

    if(cam)
    {
        glm::mat4 view(1.0f);

        if(cam->IsFirstPerson())
        {
            auto delta = glm::quat(glm::radians(cameraTransform.rotation)) * glm::vec3(0.0f, 0.0f, -1.0f);

            view = glm::lookAt(cameraTransform.position, cameraTransform.position + delta, cam->GetUp());
        }
        else
            view = glm::lookAt(cameraTransform.position, cam->GetLookAt(), cam->GetUp());

        cam->SetViewMatrix(view);
        
        Renderer::Get().GetMatrices()->GetView() = view;
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

        // Not a reference since we don't want to change the actual local transform...
        auto localTransform = transform;

        if(registry.all_of<HierarchyComponent>(entity))
            localTransform.SetTransform(GetWorldTransform(entity));

        lights.push_back(
            {
                localTransform.position,
                glm::quat(glm::radians(localTransform.rotation)) * glm::vec3(0.0f, 0.0f, -1.0f),
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

    // Fill shadowSamplers with an empty texture
    std::fill(
        shadowSamplers.begin(),
        shadowSamplers.end(),
        AssetManager::Get().Load<TextureAsset>("empty", true)->texture
    );

    auto lightsView = registry.view<LightComponent, TransformComponent>();

    for(auto entity : lightsView)
    {
        auto [light, transform] =
            lightsView.get<LightComponent, TransformComponent>(entity);

        if(light.shadowMap)
        {
            auto localTransform = transform;

            if(registry.all_of<HierarchyComponent>(entity))
                localTransform.SetTransform(GetWorldTransform(entity));

            auto delta = glm::quat(glm::radians(localTransform.rotation)) * glm::vec3(0.0f, 0.0f, -1.0f);
        
            shadows.push_back(
                {
                    light.projection *
                    glm::lookAt(localTransform.position, localTransform.position + delta, glm::vec3(0.0f, 1.0f, 0.0f)),
                    light.bias
                }
            );

            shadowSamplers[shadows.size() - 1] = light.depth;
        }
    }
}

void Scene::RenderMeshes()
{
    auto view = registry.view<TransformComponent, MeshComponent, MeshRendererComponent, PipelineComponent>();
    
    for(auto entity : view)
    {
        auto [transform, mesh, meshRenderer, pipeline] = 
                view.get<TransformComponent, MeshComponent, MeshRendererComponent, PipelineComponent>(entity);

        if(registry.all_of<RigidBodyComponent>(entity))
        {
            auto body = registry.get<RigidBodyComponent>(entity).body;

            auto position = body->GetPosition();
            auto rotation = body->GetRotation().GetEulerAngles();

            transform.position = { position.GetX(), position.GetY(), position.GetZ() };
            transform.rotation = glm::degrees(glm::vec3(rotation.GetX(), rotation.GetY(), rotation.GetZ()));
        }

        Renderer::Get().GetMatrices()->PushMatrix();
        Renderer::Get().GetMatrices()->GetModel() = GetWorldTransform(entity);

        MeshRenderPass(mesh, meshRenderer, pipeline, renderer->GetPrimaryRenderTarget());

        Renderer::Get().GetMatrices()->PopMatrix();
    }

    if(view.begin() == view.end())
        Renderer::Get().ClearRenderTarget(renderer->GetPrimaryRenderTarget());
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
            Renderer::Get().ClearRenderTarget(lightComponent.renderTarget, false);

            auto delta = glm::quat(glm::radians(lightTransform.rotation)) * glm::vec3(0.0f, 0.0f, -1.0f);

            Renderer::Get().GetMatrices()->GetView() = glm::lookAt(lightTransform.position, lightTransform.position + delta, glm::vec3(0.0f, 1.0f, 0.0f));
            Renderer::Get().GetMatrices()->GetProjection() = lightComponent.projection;

            for(auto mesh : meshesView)
            {
                auto [transform, meshComp, meshRenderer, pipeline] =
                        meshesView.get<TransformComponent, MeshComponent, MeshRendererComponent, PipelineComponent>(mesh);

                Renderer::Get().GetMatrices()->PushMatrix();
                Renderer::Get().GetMatrices()->GetModel() = GetWorldTransform(mesh);

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
    if(!mesh.model)
        return;

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

    auto gtaoResult = ApplyGTAO();

    Renderer::Get().Begin();

    RenderSky(renderTarget);

    auto defaultTexture = AssetManager::Get().Load<TextureAsset>("default", true)->texture;
    auto irradiance = defaultTexture;
    auto prefiltered = defaultTexture;
    auto brdf = defaultTexture;

    // Very bad, obviously
    auto hdriSkyView = registry.view<HDRISkyComponent>();
    auto proceduralSkyView = registry.view<ProceduralSkyComponent>();

    if(hdriSkyView.begin() != hdriSkyView.end())
    {
        auto sky = hdriSkyView.get<HDRISkyComponent>(*hdriSkyView.begin());

        irradiance = sky.asset->irradiance;
        prefiltered = sky.asset->prefiltered;
        brdf = sky.asset->brdf;
    }
    else if(proceduralSkyView.begin() != proceduralSkyView.end())
    {
        auto sky = proceduralSkyView.get<ProceduralSkyComponent>(*proceduralSkyView.begin());

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
            { 13, brdf },
            { 14, gtaoResult }
        },
        uniforms,
        renderTarget
    );

    Renderer::Get().End();

    Renderer::Get().Submit();
}

void Scene::ApplyPostProcessing(LLGL::RenderTarget* renderTarget)
{
    auto tonemapView = registry.view<TonemapComponent>();
    
    if(tonemapView->begin() == tonemapView->end())
    {
        RenderResult(renderTarget);
        return;
    }

    auto toneMapping = *tonemapView->begin();
    
    RenderResult(toneMapping.postProcessing->GetRenderTarget());

    // Problem: reflections don't affect bloom yet
    auto ssrResult = ApplySSR(toneMapping.postProcessing->GetFrame());
    auto bloomResult = ApplyBloom(toneMapping.postProcessing->GetFrame());

    // Needs some attention
    auto deferredRenderer = static_pointer_cast<DeferredRenderer>(renderer);
    
    toneMapping.postProcessing->Apply(
        {
            { 0, toneMapping.postProcessing->GetFrame() },
            { 1, bloomResult.first },
            { 2, ssrResult },
            { 3, deferredRenderer->GetAlbedo() },
            { 4, deferredRenderer->GetCombined() },
            { 5, toneMapping.lut->texture }
        },
        [&](auto commandBuffer)
        {
            toneMapping.setUniforms(commandBuffer);
            
            commandBuffer->SetUniforms(2, &bloomResult.second, sizeof(float));
        },
        renderTarget
    );
}

std::pair<LLGL::Texture*, float> Scene::ApplyBloom(LLGL::Texture* frame)
{
    auto bloomView = registry.view<BloomComponent>();

    if(bloomView->begin() == bloomView->end())
        return { AssetManager::Get().Load<TextureAsset>("empty", true)->texture, 0.0f };

    auto bloom = *bloomView->begin();

    bloom.thresholdPass->Apply(
        {
            { 0, frame }
        },
        bloom.setThresholdUniforms,
        bloom.thresholdPass->GetRenderTarget()
    );
    
    int horizontal = 1;

    auto setPingPongUniforms = [&](auto commandBuffer)
    {
        commandBuffer->SetUniforms(0, &horizontal, sizeof(int));
    };

    bloom.pingPong[1]->Apply(
        {
            { 0, bloom.thresholdPass->GetFrame() },
            { 1, bloom.sampler }
        },
        setPingPongUniforms,
        bloom.pingPong[1]->GetRenderTarget()
    );

    for(int i = 0; i < 9; i++)
    {
        horizontal = !horizontal;

        bloom.pingPong[i % 2]->Apply(
            {
                { 0, bloom.pingPong[(i + 1) % 2]->GetFrame() },
                { 1, bloom.sampler }
            },
            setPingPongUniforms,
            bloom.pingPong[i % 2]->GetRenderTarget()
        );
    }

    return { bloom.pingPong[0]->GetFrame(), bloom.strength };
}

LLGL::Texture* Scene::ApplyGTAO()
{
    auto gtaoView = registry.view<GTAOComponent>();

    if(gtaoView->begin() == gtaoView->end())
        return AssetManager::Get().Load<TextureAsset>("empty", true)->texture;

    auto gtao = *gtaoView->begin();

    auto depth = renderer->GetDepth();

    Renderer::Get().GenerateMips(depth);

    gtao.gtao->Apply(
        {
            { 0, depth }
        },
        [&](auto commandBuffer)
        {
            if(cam)
            {
                float far = cam->GetFar();
                float near = cam->GetNear();

                commandBuffer->SetUniforms(0, &far, sizeof(far));
                commandBuffer->SetUniforms(1, &near, sizeof(near));
            }

            commandBuffer->SetUniforms(2, &gtao.samples, sizeof(gtao.samples));
            commandBuffer->SetUniforms(3, &gtao.limit, sizeof(gtao.limit));
            commandBuffer->SetUniforms(4, &gtao.radius, sizeof(gtao.radius));
            commandBuffer->SetUniforms(5, &gtao.falloff, sizeof(gtao.falloff));
            commandBuffer->SetUniforms(6, &gtao.thicknessMix, sizeof(gtao.thicknessMix));
            commandBuffer->SetUniforms(7, &gtao.maxStride, sizeof(gtao.maxStride));
        }
    );

    gtao.boxBlur->Apply(
        {
            { 0, gtao.gtao->GetFrame() }
        },
        [&](auto) {}
    );

    return gtao.boxBlur->GetFrame();
}

LLGL::Texture* Scene::ApplySSR(LLGL::Texture* frame)
{
    auto ssrView = registry.view<SSRComponent>();

    if(ssrView->begin() == ssrView->end())
        return AssetManager::Get().Load<TextureAsset>("empty", true)->texture;

    auto ssr = *ssrView->begin();

    // Needs some attention
    auto deferredRenderer = static_pointer_cast<DeferredRenderer>(renderer);

    auto gNormal = deferredRenderer->GetNormal();
    auto gCombined = deferredRenderer->GetCombined();
    auto depth = deferredRenderer->GetDepth();

    ssr.ssr->Apply(
        {
            { 0, Renderer::Get().GetMatricesBuffer() },
            { 1, gNormal },
            { 2, gCombined },
            { 3, depth },
            { 4, frame }
        },
        [&](auto commandBuffer)
        {
            commandBuffer->SetUniforms(0, &ssr.maxSteps, sizeof(ssr.maxSteps));
            commandBuffer->SetUniforms(1, &ssr.maxBinarySearchSteps, sizeof(ssr.maxBinarySearchSteps));
            commandBuffer->SetUniforms(2, &ssr.rayStep, sizeof(ssr.rayStep));
        },
        nullptr,
        true
    );

    Renderer::Get().GenerateMips(ssr.ssr->GetFrame());

    return ssr.ssr->GetFrame();
}

}

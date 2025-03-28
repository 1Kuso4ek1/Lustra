#include <Scene.hpp>
#include <Entity.hpp>
#include <ScriptManager.hpp>
#include <Listener.hpp>

namespace lustra
{

Scene::Scene()
{
    Setup();
}

Scene::~Scene()
{
    EventManager::Get().RemoveListener(Event::Type::WindowResize, this);
    EventManager::Get().RemoveListener(Event::Type::Collision, this);
}

void Scene::Setup()
{
    EventManager::Get().AddListener(Event::Type::WindowResize, this);
    EventManager::Get().AddListener(Event::Type::Collision, this);

    if(!lightsBuffer)
    {
        SetupLightsBuffer();
        SetupShadowsBuffer();
    }
}

void Scene::Start()
{
    registry.view<ScriptComponent>(entt::exclude<PrefabComponent>)
        .each([&](auto entity, auto& script)
    {
        StartScript(script, { entity, this });
    });
}

void Scene::Update(float deltaTime)
{
    InputManager::Get().Update();

    registry.view<ScriptComponent>(entt::exclude<PrefabComponent>)
        .each([&](auto entity, auto& script)
    {
        UpdateScript(script, { entity, this }, deltaTime);
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

    UpdateSounds();

    Renderer::Get().Begin();

    UpdateLightsBuffer();
    UpdateShadowsBuffer();

    RenderMeshes();
    
    Renderer::Get().End();

    Renderer::Get().Submit();

    ApplyPostProcessing(renderTarget);
}

void Scene::OnEvent(Event& event)
{
    if(!isRunning)
        return;
    
    switch(event.GetType())
    {
        case Event::Type::WindowResize:
        {
            auto resizeEvent = dynamic_cast<WindowResizeEvent*>(&event);

            registry.view<ScriptComponent>(entt::exclude<PrefabComponent>)
                .each([&](auto entity, auto& script)
            {
                if(script.script)
                {
                    ScriptManager::Get().ExecuteFunction(
                        script.script,
                        "void OnWindowResize(WindowResizeEvent@)",
                        [&](auto context)
                        {
                            context->SetArgAddress(0, resizeEvent);
                        },
                        script.moduleIndex
                    );
                }
            });
        }
        break;

        case Event::Type::Collision:
        {
            auto collisionEvent = dynamic_cast<CollisionEvent*>(&event);

            registry.view<ScriptComponent>(entt::exclude<PrefabComponent>)
                .each([&](auto entity, auto& script)
            {
                if(script.script)
                {
                    ScriptManager::Get().ExecuteFunction(
                        script.script,
                        "void OnCollision(CollisionEvent@)",
                        [&](auto context)
                        {
                            context->SetArgAddress(0, collisionEvent);
                        },
                        script.moduleIndex
                    );
                }
            });
        }
        break;

        default:
            break;
    }
}

void Scene::SetUpdatePhysics(bool updatePhysics)
{
    this->updatePhysics = updatePhysics;
}

void Scene::ToggleUpdatePhysics()
{
    updatePhysics = !updatePhysics;
}

void Scene::SetIsRunning(bool running)
{
    isRunning = running;
}

void Scene::ToggleIsRunning()
{
    isRunning = !isRunning;
}

void Scene::ReparentEntity(Entity child, Entity parent)
{
    static auto removeChild = [&](Entity child, Entity parent)
    {
        if(registry.valid(parent))
        {
            auto& prevParentHierarchy = parent.GetComponent<HierarchyComponent>();

            prevParentHierarchy.children.erase(
                std::remove(prevParentHierarchy.children.begin(), 
                            prevParentHierarchy.children.end(), child),
                prevParentHierarchy.children.end()
            );
        }
    };

    auto& childHierarchy = child.GetOrAddComponent<HierarchyComponent>();
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
        auto& parentHierarchy = parent.GetOrAddComponent<HierarchyComponent>();
        parentHierarchy.children.push_back(child);

        if(child.HasComponent<TransformComponent>())
        {
            auto& childTransform = child.GetComponent<TransformComponent>();
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

Entity Scene::CloneEntity(Entity entity)
{
    auto clone = CreateEntity();

    for(auto [id, storage] : registry.storage())
    {
        if(storage.contains(entity))
            storage.push(clone, storage.value(entity));
    }

    if(clone.HasComponent<ScriptComponent>() && isRunning)
    {
        auto& script = clone.GetComponent<ScriptComponent>();
        
        if(script.script)
        {
            Multithreading::Get().AddJob(
                { nullptr, [&script, clone, this]() { StartScript(script, clone); } }
            );
        }
    }

    return clone;
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

void Scene::StartScript(ScriptComponent& script, Entity entity)
{
    if(script.script)
    {
        auto variables = ScriptManager::Get().GetGlobalVariables(script.script, script.moduleIndex);

        auto it = variables.find("Entity self");

        if(it != variables.end())
            *((Entity*)(it->second)) = entity;

        it = variables.find("Scene@ scene");

        if(it != variables.end())
            *((Scene**)(it->second)) = this;

        ScriptManager::Get().ExecuteFunction(
            script.script,
            "void Start()",
            nullptr,
            script.moduleIndex
        );
    }
}

void Scene::UpdateScript(ScriptComponent& script, Entity entity, float deltaTime)
{
    if(script.script)
    {
        ScriptManager::Get().ExecuteFunction(
            script.script,
            "void Update(float)",
            [&](auto context)
            {
                context->SetArgFloat(0, deltaTime);
            },
            script.moduleIndex
        );
    }
}

void Scene::SetupLightsBuffer()
{
    static const uint64_t maxLights = 128;

    LLGL::BufferDescriptor lightBufferDesc = LLGL::ConstantBufferDesc(maxLights * sizeof(Light));
    
    lightsBuffer = Renderer::Get().CreateBuffer("sceneLightBuffer", lightBufferDesc);

    lights.reserve(maxLights);
}

void Scene::SetupShadowsBuffer()
{
    static const uint64_t maxShadows = 4;

    LLGL::BufferDescriptor shadowBufferDesc = LLGL::ConstantBufferDesc(maxShadows * sizeof(Shadow));

    shadowsBuffer = Renderer::Get().CreateBuffer("sceneShadowBuffer", shadowBufferDesc);

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

void Scene::UpdateSounds()
{
    auto soundsView = registry.view<SoundComponent, TransformComponent>(entt::exclude<PrefabComponent>);
    
    for(auto entity : soundsView)
    {
        auto [sound, transform] =
            soundsView.get<SoundComponent, TransformComponent>(entity);

        auto worldTransform = transform;

        if(registry.all_of<HierarchyComponent>(entity))
            worldTransform.SetTransform(GetWorldTransform(entity));

        if(sound.sound)
        {
            if(sound.sound->sound.GetSound().get()) // ??? :/
            {
                sound.sound->sound.SetPosition(worldTransform.position);
                sound.sound->sound.SetOrientation(glm::radians(worldTransform.rotation));
            }
        }
    }
}

void Scene::SetupCamera()
{
    auto cameraView = registry.view<TransformComponent, CameraComponent>(entt::exclude<PrefabComponent>);

    TransformComponent cameraTransform;

    camera = nullptr;

    for(auto entity : cameraView)
    {
        auto [transform, cam] = cameraView.get<TransformComponent, CameraComponent>(entity);

        if(/* !camera ||  */cam.active)
        {
            camera = &cam.camera;
            cameraTransform = transform;
        
            if(registry.all_of<HierarchyComponent>(entity))
            {
                auto rotation = cameraTransform.rotation;

                cameraTransform.SetTransform(GetWorldTransform(entity));

                cameraTransform.rotation = rotation;
            }

            cameraPosition = cameraTransform.position;

            Listener::Get().SetPosition(cameraPosition);
            Listener::Get().SetOrientation(glm::radians(cameraTransform.rotation));
        }
    }

    if(camera)
    {
        glm::mat4 view(1.0f);

        if(camera->IsFirstPerson())
        {
            auto delta = glm::quat(glm::radians(cameraTransform.rotation)) * glm::vec3(0.0f, 0.0f, -1.0f);

            view = glm::lookAt(cameraTransform.position, cameraTransform.position + delta, camera->GetUp());
        }
        else
            view = glm::lookAt(cameraTransform.position, camera->GetLookAt(), camera->GetUp());

        camera->SetViewMatrix(view);
        
        Renderer::Get().GetMatrices()->GetView() = view;
        Renderer::Get().GetMatrices()->GetProjection() = camera->GetProjectionMatrix();
    }
}

void Scene::SetupLights()
{
    lights.clear();

    auto lightsView = registry.view<LightComponent, TransformComponent>(entt::exclude<PrefabComponent>);

    for(auto entity : lightsView)
    {
        auto [light, transform] = lightsView.get<LightComponent, TransformComponent>(entity);

        // Not a reference since we don't want to change the actual local transform...
        auto localTransform = transform;

        if(registry.all_of<HierarchyComponent>(entity))
            localTransform.SetTransform(GetWorldTransform(entity));

        lights.push_back(
            {
                (int)light.shadowMap,
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

    auto lightsView = registry.view<LightComponent, TransformComponent>(entt::exclude<PrefabComponent>);

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
    bool empty = true;

    auto view =
        registry.view<
            TransformComponent,
            MeshComponent,
            MeshRendererComponent,
            PipelineComponent
        >(entt::exclude<PrefabComponent>);
    
    for(auto entity : view)
    {
        auto [transform, mesh, meshRenderer, pipeline] = 
                view.get<TransformComponent, MeshComponent, MeshRendererComponent, PipelineComponent>(entity);

        if(registry.all_of<RigidBodyComponent>(entity))
        {
            auto body = registry.get<RigidBodyComponent>(entity).body;

            if(transform.overridePhysics)
            {
                auto bodyId = body->GetID();

                auto position = transform.position;
                auto rotation = glm::quat(glm::radians(transform.rotation));

                PhysicsManager::Get().GetBodyInterface().SetPositionAndRotation(
                    bodyId,
                    { position.x, position.y, position.z },
                    { rotation.x, rotation.y, rotation.z, rotation.w },
                    JPH::EActivation::Activate
                );

                body->SetLinearVelocity({ 0.0f, 0.0f, 0.0f });
                body->SetAngularVelocity({ 0.0f, 0.0f, 0.0f });
            }
            else
            {
                auto position = body->GetPosition();
                auto rotation = body->GetRotation().GetEulerAngles();

                transform.position = { position.GetX(), position.GetY(), position.GetZ() };
                transform.rotation = glm::degrees(glm::vec3(rotation.GetX(), rotation.GetY(), rotation.GetZ()));
            }
        }

        if(!mesh.drawable)
            continue;

        Renderer::Get().GetMatrices()->PushMatrix();
        Renderer::Get().GetMatrices()->GetModel() = GetWorldTransform(entity);

        MeshRenderPass(mesh, meshRenderer, pipeline, DeferredRenderer::Get().GetPrimaryRenderTarget());

        Renderer::Get().GetMatrices()->PopMatrix();

        empty = false;
    }

    if(empty)
        Renderer::Get().ClearRenderTarget(DeferredRenderer::Get().GetPrimaryRenderTarget());
}

void Scene::RenderToShadowMap()
{
    Renderer::Get().Begin();

    auto meshesView =
        registry.view<
            TransformComponent,
            MeshComponent,
            MeshRendererComponent,
            PipelineComponent
        >(entt::exclude<PrefabComponent>);

    auto lightsView = registry.view<LightComponent, TransformComponent>(entt::exclude<PrefabComponent>);

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
    auto hdriSkyView = registry.view<MeshComponent, HDRISkyComponent>(entt::exclude<PrefabComponent>);

    if(hdriSkyView.begin() != hdriSkyView.end())
    {
        auto [mesh, sky] = 
            hdriSkyView.get<MeshComponent, HDRISkyComponent>(*hdriSkyView.begin());

        HDRISkyRenderPass(mesh, sky, renderTarget);

        return;
    }

    auto proceduralSkyView = registry.view<MeshComponent, ProceduralSkyComponent>(entt::exclude<PrefabComponent>);

    if(proceduralSkyView.begin() != proceduralSkyView.end())
    {
        auto [mesh, sky] = proceduralSkyView.get<MeshComponent, ProceduralSkyComponent>(*proceduralSkyView.begin());

        ProceduralSkyRenderPass(mesh, sky, renderTarget);
    }
}

void Scene::MeshRenderPass(
    const MeshComponent& mesh,
    const MeshRendererComponent& meshRenderer,
    const PipelineComponent& pipeline,
    LLGL::RenderTarget* renderTarget
)
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

                float time = global::appTimer.GetElapsedSeconds();

                commandBuffer->SetUniforms(13, &time, sizeof(time));

                mesh.model->meshes[i]->Draw(commandBuffer);
            },
            pipeline.pipeline,
            DeferredRenderer::Get().GetPrimaryRenderTarget()
        );
    }
}

void Scene::ShadowRenderPass(const LightComponent& light, const MeshComponent& mesh)
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

void Scene::ProceduralSkyRenderPass(
    const MeshComponent& mesh,
    const ProceduralSkyComponent& sky,
    LLGL::RenderTarget* renderTarget
)
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

void Scene::HDRISkyRenderPass(
    const MeshComponent& mesh,
    const HDRISkyComponent& sky,
    LLGL::RenderTarget* renderTarget
)
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
    auto uniforms = [&](auto commandBuffer)
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
        auto& sky = hdriSkyView.get<HDRISkyComponent>(*hdriSkyView.begin());

        irradiance = sky.asset->irradiance;
        prefiltered = sky.asset->prefiltered;
        brdf = sky.asset->brdf;
    }
    else if(proceduralSkyView.begin() != proceduralSkyView.end())
    {
        auto& sky = proceduralSkyView.get<ProceduralSkyComponent>(*proceduralSkyView.begin());

        irradiance = sky.asset->irradiance;
        prefiltered = sky.asset->prefiltered;
        brdf = sky.asset->brdf;
    }

    DeferredRenderer::Get().Draw(
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

    auto& toneMapping = *tonemapView->begin();
    
    if(!toneMapping.postProcessing)
    {
        RenderResult(renderTarget);
        return;
    }
    
    RenderResult(toneMapping.postProcessing->GetRenderTarget());

    // Problem: reflections don't affect bloom yet
    auto ssrResult = ApplySSR(toneMapping.postProcessing->GetFrame());
    auto bloomResult = ApplyBloom(toneMapping.postProcessing->GetFrame());
    
    toneMapping.postProcessing->Apply(
        {
            { 0, toneMapping.postProcessing->GetFrame() },
            { 1, bloomResult.first },
            { 2, ssrResult },
            { 3, DeferredRenderer::Get().GetAlbedo() },
            { 4, DeferredRenderer::Get().GetCombined() },
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

    auto& bloom = *bloomView->begin();

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

    auto& gtao = *gtaoView->begin();

    auto depth = DeferredRenderer::Get().GetDepth();

    Renderer::Get().GenerateMips(depth);

    gtao.gtao->Apply(
        {
            { 0, depth }
        },
        [&](auto commandBuffer)
        {
            if(camera)
            {
                float far = camera->GetFar();
                float near = camera->GetNear();

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

    auto& ssr = *ssrView->begin();

    auto gNormal = DeferredRenderer::Get().GetNormal();
    auto gCombined = DeferredRenderer::Get().GetCombined();
    auto depth = DeferredRenderer::Get().GetDepth();

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

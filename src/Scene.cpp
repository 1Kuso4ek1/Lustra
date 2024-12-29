#include <Scene.hpp>
#include <Entity.hpp>

namespace dev
{

Scene::Scene(std::shared_ptr<RendererBase> renderer)
    : renderer(renderer)
{
    EventManager::Get().AddListener(Event::Type::WindowResize, this);
}

void Scene::SetRenderer(std::shared_ptr<RendererBase> renderer)
{
    this->renderer = renderer;
}

void Scene::Start()
{
    if(!lightsBuffer)
        SetupLightsBuffer();

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

void Scene::Draw()
{
    SetupCamera();
    SetupLights();

    auto view = registry.view<TransformComponent, MeshComponent, MeshRendererComponent, PipelineComponent>();

    Renderer::Get().Begin();

    Renderer::Get().RenderPass(
        [&](auto commandBuffer)
        {
            commandBuffer->UpdateBuffer(*lightsBuffer, 0, lights.data(), lights.size() * sizeof(Light));
        }, {}, [](auto){}, nullptr
    );

    for(auto entity : view)
    {
        auto [transform, mesh, meshRenderer, pipeline] = 
                view.get<TransformComponent, MeshComponent, MeshRendererComponent, PipelineComponent>(entity);

        Renderer::Get().GetMatrices()->PushMatrix();
        Renderer::Get().GetMatrices()->GetModel() = transform.GetTransform();

        RenderMeshes(mesh, meshRenderer, pipeline);

        Renderer::Get().GetMatrices()->PopMatrix();
    }
    
    Renderer::Get().End();

    Renderer::Get().Submit();

    ApplyPostProcessing();
}

void Scene::OnEvent(Event& event)
{
    if(event.GetType() == Event::Type::WindowResize)
    {
        auto resizeEvent = dynamic_cast<WindowResizeEvent*>(&event);
        

    }
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

void Scene::RenderMeshes(MeshComponent mesh, MeshRendererComponent meshRenderer, PipelineComponent pipeline)
{
    for(size_t i = 0; i < mesh.meshes.size(); i++)
    {
        auto material = TextureManager::Get().GetDefaultTexture();

        if(meshRenderer.materials.size() > i)
            material = meshRenderer.materials[i].get();

        Renderer::Get().RenderPass(
            [&](auto commandBuffer)
            {
                mesh.meshes[i]->BindBuffers(commandBuffer);
            },
            {
                { 0, Renderer::Get().GetMatricesBuffer() },
                { 1, material->texture },
                { 2, TextureManager::Get().GetAnisotropySampler() }
            },
            [&](auto commandBuffer)
            {
                mesh.meshes[i]->Draw(commandBuffer);
            },
            pipeline.pipeline,
            renderer->GetPrimaryRenderTarget()
        );
    }
}

void Scene::ApplyPostProcessing()
{
    auto acesView = registry.view<ACESTonemappingComponent>();

    auto uniforms = [&](auto commandBuffer)
    {
        auto numLights = lights.size();
        commandBuffer->SetUniforms(0, &numLights, sizeof(numLights));
        commandBuffer->SetUniforms(1, &cameraPosition, sizeof(cameraPosition));
    };

    if(acesView->begin() == acesView->end())
    {
        renderer->Draw(
            { { 3, lightsBuffer } },
            uniforms
        );
        return;
    }

    auto postProcessing = *acesView->begin();

    renderer->Draw(
        { { 3, lightsBuffer } },
        uniforms,
        postProcessing.postProcessing->GetRenderTarget()
    );

    postProcessing.postProcessing->Apply(
        {
            { 0, postProcessing.postProcessing->GetFrame() },
            { 1, TextureManager::Get().GetAnisotropySampler() }
        },
        postProcessing.setUniforms,
        Renderer::Get().GetSwapChain()
    );
}
    
}

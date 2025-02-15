JPH::Body@ rigidBody;
TransformComponent@ transform;
TransformComponent@ cameraTransform;

void Start(Scene@ scene, Entity entity)
{
    Log::Write(format("{}\n", entity.GetNameComponent().name));

    //@rigidBody = @entity.GetRigidBodyComponent().body;
    @transform = @entity.GetTransformComponent();

    @cameraTransform = @scene.GetEntity("Camera").GetTransformComponent();

    InputManager::MapKeyboardAction("Up", Keyboard::Key::Space);
    InputManager::MapKeyboardAction("Up", Keyboard::Key::Up);
    InputManager::MapKeyboardAction("Up", Keyboard::Key::W);

    auto@ material = entity.GetMeshRendererComponent();
    material.at(0).get().albedo = glm::vec4(1.0, 0.0, 0.0, 1.0);

    entity.GetMeshComponent().model = AssetManager::LoadModel("cube", true);
}

void Update(Scene@ scene, Entity entity, float deltaTime)
{
    if(InputManager::IsActionPressed("Up"))
        transform.position.y += 1.0f * deltaTime;
}

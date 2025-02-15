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

    TextureAssetPtr asset = AssetManager::LoadTexture("default", true);
    TextureAsset@ textureAsset = asset.get();
    textureAsset.print();
}

void Update(Scene@ scene, Entity entity, float deltaTime)
{
    if(InputManager::IsActionPressed("Up"))
        transform.position.y += 1.0f * deltaTime;
}

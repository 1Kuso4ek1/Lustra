Entity self;
Scene@ scene;

JPH::Body@ rigidBody;
TransformComponent@ transform;
TransformComponent@ cameraTransform;

void Start()
{
    Log::Write(format("{}\n", self.GetNameComponent().name));

    //@rigidBody = @self.GetRigidBodyComponent().body;
    @transform = @self.GetTransformComponent();

    @cameraTransform = @scene.GetEntity("Camera").GetTransformComponent();

    InputManager::MapKeyboardAction("Up", Keyboard::Key::Space);
    InputManager::MapKeyboardAction("Up", Keyboard::Key::Up);
    InputManager::MapKeyboardAction("Up", Keyboard::Key::W);

    auto@ material = self.GetMeshRendererComponent();
    material.at(0).get().albedo = glm::vec4(1.0, 0.0, 0.0, 1.0);

    self.GetMeshComponent().model = AssetManager::LoadModel("cube", true);
}

void Update(float deltaTime)
{
    if(InputManager::IsActionPressed("Up"))
        transform.position.y += 1.0f * deltaTime;
}

void OnWindowResize(WindowResizeEvent@ event)
{
    Log::Write(format("Size: {}x{}\n", event.GetSize().width, event.GetSize().height));
}

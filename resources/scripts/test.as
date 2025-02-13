JPH::Body@ rigidBody;
TransformComponent@ transform;
TransformComponent@ cameraTransform;

void Start(Scene@ scene, Entity entity)
{
    Log::Write(format("{}\n", entity.GetNameComponent().name));

    @rigidBody = @entity.GetRigidBodyComponent().body;
    @transform = @entity.GetTransformComponent();

    @cameraTransform = @scene.GetEntity("Camera").GetTransformComponent();
}

void Update(Scene@ scene, Entity entity, float deltaTime)
{
    transform.rotation = cameraTransform.rotation;

    rigidBody.SetRotation(glm::quat(glm::radians(transform.rotation)));
}

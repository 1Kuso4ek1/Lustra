JPH::Body@ rigidBody;
TransformComponent@ transform;

void Start(Entity entity)
{
    Log::Write(format("{}\n", entity.GetNameComponent().name));

    @rigidBody = @entity.GetRigidBodyComponent().body;
    @transform = @entity.GetTransformComponent();
}

void Update(Entity entity, float deltaTime)
{
    transform.rotation.y += 50.0f * deltaTime;

    rigidBody.SetRotation(glm::quat(glm::radians(transform.rotation)));
}

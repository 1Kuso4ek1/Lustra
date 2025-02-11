void Start(Entity entity)
{
    Log::Write(format("{}\n", entity.GetNameComponent().name));
}

void Update(Entity entity, float deltaTime)
{
    entity.GetTransformComponent().rotation.y += 50.0f * deltaTime;
}

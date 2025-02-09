void Start(Entity entity)
{
    Log::Write(format("{}\n", entity.GetNameComponent().name));
}

void Update(Entity entity, float deltaTime)
{
    if(Keyboard::IsKeyPressed(Keyboard::Key::V))
    {
        Log::Write(format("deltaTime: {}\n", deltaTime));
        Log::Write(format("{}\n", entity.GetNameComponent().name));
    }

    entity.GetTransformComponent().rotation.y += 5.0f * deltaTime;
}

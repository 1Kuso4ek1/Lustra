void Start(dev::Entity@ entity)
{
    Log::Write(format("{}\n", entity.GetNameComponent().name));
}

void Update(dev::Entity@ entity, float deltaTime)
{
    if(dev::Keyboard::IsKeyPressed(dev::Keyboard::Key::V))
        Log::Write(format("deltaTime: {}\n", deltaTime));
}

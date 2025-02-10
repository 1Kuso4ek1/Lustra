void Start(Entity entity)
{
    Log::Write(format("{}\n", entity.GetNameComponent().name));

    glm::vec2 vec(1.0f);

    Log::Write(format("glm::vec2({}, {})\n", vec.x, vec.y));
    
    glm::vec2 anotherVec(2.0f);
    glm::vec2 res = vec + anotherVec;
    Log::Write(format("glm::vec2({}, {})\n", res.x, res.y));
    res += vec;
    Log::Write(format("glm::vec2({}, {})\n", res.x, res.y));
}

void Update(Entity entity, float deltaTime)
{
    if(Keyboard::IsKeyPressed(Keyboard::Key::V))
    {
        Log::Write(format("deltaTime: {}\n", deltaTime));
        Log::Write(format("{}\n", entity.GetNameComponent().name));
    }

    entity.GetTransformComponent().rotation.y += 50.0f * deltaTime;
}

TransformComponent@ transform = null;
Camera@ camera = null;
float maxSpeed = 3.0f;
float currentSpeed = 0.0f;

glm::vec3 movement(0.0f);

void Start(Entity entity)
{
    @transform = @entity.GetTransformComponent();
    @camera = @entity.GetCameraComponent().camera;
}

void Update(Entity entity, float deltaTime)
{
    if(Mouse::IsButtonPressed(Mouse::Button::Right)/*  && canMoveCamera */)
    {
        Mouse::SetCursorVisible(false);

        auto rotation = glm::quat(glm::radians(transform.rotation));

        glm::vec3 input = glm::vec3(0.0f);

        if(Keyboard::IsKeyPressed(Keyboard::Key::W))
            input -= rotation * glm::vec3(0.0f, 0.0f, 1.0f);
        if(Keyboard::IsKeyPressed(Keyboard::Key::S))
            input += rotation * glm::vec3(0.0f, 0.0f, 1.0f);
        if(Keyboard::IsKeyPressed(Keyboard::Key::A))
            input -= rotation * glm::vec3(1.0f, 0.0f, 0.0f);
        if(Keyboard::IsKeyPressed(Keyboard::Key::D))
            input += rotation * glm::vec3(1.0f, 0.0f, 0.0f);

        float lerpSpeed = glm::clamp(deltaTime * 5.0f, 0.0f, 1.0f);

        if(glm::length(input) > 0.1f)
        {
            currentSpeed = glm::mix(currentSpeed, maxSpeed, lerpSpeed);
            movement = input;
        }
        else
            currentSpeed = glm::mix(currentSpeed, 0.0f, lerpSpeed);

        if(currentSpeed > 0.0f)
            transform.position += glm::normalize(movement) * deltaTime * currentSpeed;

        glm::vec2 center(960, 540);
        glm::vec2 delta = center - Mouse::GetPosition();

        transform.rotation.x += delta.y / 100.0f;
        transform.rotation.y += delta.x / 100.0f;

        transform.rotation.x = glm::clamp(transform.rotation.x, -89.0f, 89.0f);

        Mouse::SetPosition(center);
    }
    else
        Mouse::SetCursorVisible();
}

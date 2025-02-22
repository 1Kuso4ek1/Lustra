Entity self;
Scene@ scene;

TransformComponent@ transform;

void Start()
{
    @transform = @self.GetTransformComponent();
}

void Update(float deltaTime)
{
    transform.overridePhysics = true;
    transform.rotation.y += 5.0f * deltaTime;
}

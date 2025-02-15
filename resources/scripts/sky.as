void Start(Scene@ scene, Entity entity)
{
    entity.GetHDRISkyComponent().environmentMap = AssetManager::LoadTexture("hdri/sky5.hdr", true);
    entity.GetHDRISkyComponent().Build();
}

void Update(Scene@ scene, Entity entity, float deltaTime)
{
    
}

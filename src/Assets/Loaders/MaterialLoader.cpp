#include <MaterialLoader.hpp>
#include <AssetManager.hpp>
#include <EventManager.hpp>

#include <fstream>

namespace dev
{

AssetPtr MaterialLoader::Load(const std::filesystem::path& path)
{
    if(!defaultMaterial)
        LoadDefaultData();
    
    if(path.filename() == "default")
        return defaultMaterial;

    std::ofstream file(path.string());

    auto material = std::make_shared<MaterialAsset>();

    material->albedo.texture = defaultTexture;
    material->albedo.texture = defaultTexture;
    material->normal.texture = defaultTexture;
    material->metallic.texture = defaultTexture;
    material->roughness.texture = defaultTexture;
    material->ao.texture = defaultTexture;
    material->emission.texture = defaultTexture;

    material->loaded = true;

    EventManager::Get().Dispatch(std::make_unique<AssetLoadedEvent>(material));

    return material;
}

void MaterialLoader::LoadDefaultData()
{
    defaultTexture = AssetManager::Get().Load<TextureAsset>("default", true);
    defaultMaterial = std::make_shared<MaterialAsset>();

    defaultMaterial->albedo.texture = defaultTexture;
    defaultMaterial->normal.texture = defaultTexture;
    defaultMaterial->metallic.texture = defaultTexture;
    defaultMaterial->roughness.texture = defaultTexture;
    defaultMaterial->ao.texture = defaultTexture;
    defaultMaterial->emission.texture = defaultTexture;
    
    defaultMaterial->loaded = true;
}

}

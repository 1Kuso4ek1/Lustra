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
    material->albedo = { 1.0f, 1.0f, 1.0f, 1.0f };

    material->loaded = true;

    EventManager::Get().Dispatch(std::make_unique<AssetLoadedEvent>(material));

    return material;
}

void MaterialLoader::LoadDefaultData()
{
    defaultTexture = AssetManager::Get().Load<TextureAsset>("default", true);
    defaultMaterial = std::make_shared<MaterialAsset>();

    defaultMaterial->albedo.texture = defaultTexture;
    defaultMaterial->albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
}

}

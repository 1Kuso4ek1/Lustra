#include <MaterialLoader.hpp>
#include <AssetManager.hpp>

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

    return std::make_shared<MaterialAsset>();
}

void MaterialLoader::LoadDefaultData()
{
    defaultMaterial = std::make_shared<MaterialAsset>();

    auto defaultTexture = AssetManager::Get().Load<TextureAsset>("default", true);

    defaultMaterial->albedo.texture = defaultTexture;
    defaultMaterial->albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
}

}

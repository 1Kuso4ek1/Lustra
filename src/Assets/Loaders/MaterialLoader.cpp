#include <MaterialLoader.hpp>
#include <AssetManager.hpp>

#include <fstream>

namespace dev
{

AssetPtr MaterialLoader::Load(const std::filesystem::path& path)
{
    if(!defaultMaterial)
        LoadDefaultData();
    
    if(path.empty())
        return defaultMaterial;

    std::ofstream file(path.string());

    return std::make_shared<MaterialAsset>();
}

void MaterialLoader::LoadDefaultData()
{
    defaultMaterial = std::make_shared<MaterialAsset>();

    auto defaultTexture = AssetManager::Get().Load<TextureAsset>("");

    defaultMaterial->albedo = defaultTexture;
}

}

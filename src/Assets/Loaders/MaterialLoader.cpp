#include <MaterialLoader.hpp>

namespace dev
{

AssetPtr MaterialLoader::Load(const std::filesystem::path& path)
{
    if(!defaultMaterial)
        LoadDefaultData();
    
    if(path.empty())
        return defaultMaterial;

    return std::make_shared<MaterialAsset>();
}

void MaterialLoader::LoadDefaultData()
{
    defaultMaterial = std::make_shared<MaterialAsset>();
}

}

#include <cereal/archives/json.hpp>

#include <Serialize.hpp>
#include <MaterialLoader.hpp>
#include <AssetManager.hpp>
#include <EventManager.hpp>

#include <fstream>

namespace lustra
{

AssetPtr MaterialLoader::Load(const std::filesystem::path& path)
{
    if(!defaultMaterial)
        LoadDefaultData();
    
    if(path.filename() == "default")
        return defaultMaterial;

    std::ifstream file(path.string());

    if(!file.is_open())
    {
        LLGL::Log::Errorf(
            LLGL::Log::ColorFlags::StdError,
            "Failed to load material: %s\n",
            path.string().c_str()
        );

        return defaultMaterial;
    }

    auto material = std::make_shared<MaterialAsset>();
    
    cereal::JSONInputArchive archive(file);

    archive(*material);

    material->loaded = true;

    EventManager::Get().Dispatch(std::make_unique<AssetLoadedEvent>(material));

    return material;
}

void MaterialLoader::Write(const AssetPtr& asset, const std::filesystem::path& path)
{
    auto material = std::static_pointer_cast<MaterialAsset>(asset);

    std::ofstream file(path);

    cereal::JSONOutputArchive archive(file);

    archive(*material);
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

    defaultMaterial->metallic = glm::vec4(0.0f);
    defaultMaterial->emission = glm::vec4(0.0f);

    defaultMaterial->loaded = true;
}

}

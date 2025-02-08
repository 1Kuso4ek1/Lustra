#pragma once
#include <memory>
#include <unordered_map>

namespace dev
{

struct Asset
{
    enum class Type
    {
        Unknown,
        Texture,
        Material,
        Model,
        Environment,
        Script
    };

    Asset(Type type) : type(type) {}
    virtual ~Asset() = default;

    Type type = Type::Unknown;

    bool loaded = false;
};

using AssetPtr = std::shared_ptr<Asset>;

inline Asset::Type GetAssetType(const std::string& extension)
{
    static const std::unordered_map<std::string, Asset::Type> extensionMap =
    {
        { ".png", Asset::Type::Texture },
        { ".jpg", Asset::Type::Texture },
        { ".jpeg", Asset::Type::Texture },
        { ".bmp", Asset::Type::Texture },
        { ".tga", Asset::Type::Texture },
        { ".hdr", Asset::Type::Texture },

        { ".mtl", Asset::Type::Material },

        { ".fbx", Asset::Type::Model },
        { ".obj", Asset::Type::Model },
        { ".gltf", Asset::Type::Model },
        { ".glb", Asset::Type::Model },
    };

    auto it = extensionMap.find(extension);
    if(it != extensionMap.end())
        return it->second;

    return Asset::Type::Unknown; 
}

}

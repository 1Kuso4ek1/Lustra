#pragma once
#include <Event.hpp>

#include <memory>
#include <unordered_map>
#include <filesystem>

namespace lustra
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
        Script,
        VertexShader,
        FragmentShader,
        Scene,
        Sound
    };

    Asset(Type type) : type(type) {}
    virtual ~Asset() = default;

    Type type = Type::Unknown;

    bool loaded = false;

    std::filesystem::path path;
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

        { ".mat", Asset::Type::Material },

        { ".fbx", Asset::Type::Model },
        { ".obj", Asset::Type::Model },
        { ".gltf", Asset::Type::Model },
        { ".glb", Asset::Type::Model },
        { ".dae", Asset::Type::Model },

        { ".as", Asset::Type::Script },

        { ".json", Asset::Type::Scene },
        { ".scn", Asset::Type::Scene },

        { ".mp3", Asset::Type::Sound },
        { ".wav", Asset::Type::Sound },
        { ".ogg", Asset::Type::Sound },

        { ".vert", Asset::Type::VertexShader },
        { ".vs", Asset::Type::VertexShader },

        { ".frag", Asset::Type::FragmentShader },
        { ".fs", Asset::Type::FragmentShader }
    };

    auto it = extensionMap.find(extension);
    if(it != extensionMap.end())
        return it->second;

    return Asset::Type::Unknown; 
}

class AssetLoadedEvent : public Event
{
public:
    AssetLoadedEvent(AssetPtr asset) : Event(Type::AssetLoaded), asset(asset) {}

    AssetPtr GetAsset() const { return asset; }

private:
    AssetPtr asset;
};

}

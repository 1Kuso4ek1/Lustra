#pragma once
#include <memory>

namespace dev
{

struct Asset
{
    enum class Type
    {
        Unknown,
        Texture,
        Material,
        Model
    };

    Asset(Type type) : type(type) {}
    virtual ~Asset() = default;

    Type type = Type::Unknown;
};

using AssetPtr = std::shared_ptr<Asset>;

}

#pragma once

namespace dev
{

struct Asset
{
    enum class Type
    {
        Unknown,
        Texture,
        Model
    };

    Asset(Type type) : type(type) {}
    virtual ~Asset() = default;

    Type type = Type::Unknown;
};

}

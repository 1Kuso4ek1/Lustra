#pragma once
#include <TextureAsset.hpp>

#include <glm/glm.hpp>

namespace dev
{

struct MaterialAsset : public Asset
{
    struct Property
    {
        enum class Type { Color, Texture };

        Type type = Type::Color;

        Property& operator=(const glm::vec4& value)
        {
            type = Type::Color;
            this->value = value;

            return *this;
        }

        Property& operator=(const TextureAssetPtr& texture)
        {
            type = Type::Texture;
            this->texture = texture;

            return *this;
        }

        glm::vec4 value{ 1.0f };
        TextureAssetPtr texture;
    };

    MaterialAsset() : Asset(Type::Material) {}

    Property albedo;
    Property normal;
    Property metallic;
    Property roughness;
    Property ao;
    Property emission;
};

using MaterialAssetPtr = std::shared_ptr<MaterialAsset>;

}

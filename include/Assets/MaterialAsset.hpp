#pragma once
#include <TextureAsset.hpp>

#include <LLGL/CommandBuffer.h>

#include <glm/glm.hpp>

namespace dev
{

struct MaterialAsset : public Asset
{
    struct Property
    {
        enum class Type { Color, Texture };

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

        Type type = Type::Color;

        glm::vec4 value{ 1.0f };
        TextureAssetPtr texture;
    };

    MaterialAsset() : Asset(Type::Material) {}

    void SetUniforms(LLGL::CommandBuffer* commandBuffer)
    {
        commandBuffer->SetUniforms(0, &albedo.type, sizeof(albedo.type));

        if(albedo.type == Property::Type::Color)
            commandBuffer->SetUniforms(1, &albedo.value, sizeof(albedo.value));
    }

    Property albedo;
    Property normal;
    Property metallic;
    Property roughness;
    Property ao;
    Property emission;
};

using MaterialAssetPtr = std::shared_ptr<MaterialAsset>;

}

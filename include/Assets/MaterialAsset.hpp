#pragma once
#include <TextureAsset.hpp>
#include <AssetManager.hpp>

#include <LLGL/CommandBuffer.h>

#include <glm/glm.hpp>

namespace dev
{

struct MaterialAsset : public Asset
{
    struct Property
    {
        enum class Type { Color, Texture };

        Property()
        {
            texture = AssetManager::Get().Load<TextureAsset>("default", true);
        }

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

        template<class Archive>
        void save(Archive& archive) const
        {
            archive(type, value, texture ? texture->path.string() : "default");
        }

        template<class Archive>
        void load(Archive& archive)
        {
            std::string texturePath;

            archive(type, value, texturePath);

            texture = AssetManager::Get().Load<TextureAsset>(texturePath);
        }

        Type type = Type::Color;

        glm::vec4 value{ 1.0f };
        TextureAssetPtr texture;
    };

    MaterialAsset() : Asset(Type::Material)
    {
        roughness = glm::vec4(0.5f);
        metallic = glm::vec4(0.5f);
        emission = glm::vec4(0.0f);
    }

    void SetUniforms(LLGL::CommandBuffer* commandBuffer)
    {
        commandBuffer->SetUniforms(0, &albedo.type, sizeof(albedo.type));
        commandBuffer->SetUniforms(2, &normal.type, sizeof(normal.type));
        commandBuffer->SetUniforms(3, &metallic.type, sizeof(metallic.type));
        commandBuffer->SetUniforms(5, &roughness.type, sizeof(roughness.type));
        commandBuffer->SetUniforms(7, &ao.type, sizeof(ao.type));
        commandBuffer->SetUniforms(8, &emission.type, sizeof(emission.type));

        if(albedo.type == Property::Type::Color)
            commandBuffer->SetUniforms(1, &albedo.value, sizeof(albedo.value));

        if(metallic.type == Property::Type::Color)
            commandBuffer->SetUniforms(4, &metallic.value.x, sizeof(metallic.value.x));

        if(roughness.type == Property::Type::Color)
            commandBuffer->SetUniforms(6, &roughness.value.x, sizeof(roughness.value.x));

        if(emission.type == Property::Type::Color)
            commandBuffer->SetUniforms(9, &emission.value, sizeof(emission.value) - sizeof(emission.value.x));

        commandBuffer->SetUniforms(10, &emissionStrength, sizeof(emissionStrength));
        commandBuffer->SetUniforms(11, &uvScale, sizeof(uvScale));
    }

    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(albedo, normal, metallic, roughness, ao, emission, emissionStrength, uvScale);
    }

    Property albedo;
    Property normal;
    Property metallic;
    Property roughness;
    Property ao;
    Property emission;

    float emissionStrength = 1.0f;
    glm::vec2 uvScale = { 1.0f, 1.0f };
};

using MaterialAssetPtr = std::shared_ptr<MaterialAsset>;

}

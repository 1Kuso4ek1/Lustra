#pragma once
#include <Asset.hpp>

#include <LLGL/ImageFlags.h>
#include <LLGL/Sampler.h>
#include <LLGL/Texture.h>

namespace dev
{

struct TextureAsset : public Asset
{
    TextureAsset(const LLGL::TextureDescriptor& textureDesc, const LLGL::ImageView* imageView)
        : textureDesc(textureDesc)
    {
        if(imageView)
            this->imageView = *imageView;
    }

    TextureAsset(LLGL::Texture* texture = {}) : texture(texture)
    {
        textureDesc.type = LLGL::TextureType::Texture2D;
        textureDesc.format = LLGL::Format::RGBA8UNorm;
        textureDesc.miscFlags = LLGL::MiscFlags::GenerateMips;
        
        imageView.format = LLGL::ImageFormat::RGBA;
        imageView.dataType = LLGL::DataType::UInt8;
    };

    LLGL::Texture* texture{};
    LLGL::Sampler* sampler{};

    LLGL::TextureDescriptor textureDesc;
    LLGL::ImageView imageView;
};

}

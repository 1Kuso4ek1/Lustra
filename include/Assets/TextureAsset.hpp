#pragma once
#include <Asset.hpp>

#include <LLGL/ImageFlags.h>
#include <LLGL/Sampler.h>
#include <LLGL/Texture.h>

#ifdef __linux__
    // Workaround for an issue with "unknown type name 'Status'" in the glxext.h
    #define Status int
#endif

#include <LLGL/Backend/OpenGL/NativeHandle.h>

namespace lustra
{

struct TextureAsset final : Asset
{
    TextureAsset(const LLGL::TextureDescriptor& textureDesc, const LLGL::ImageView* imageView)
        : Asset(Type::Texture), textureDesc(textureDesc)
    {
        if(imageView)
            this->imageView = *imageView;
    }

    explicit TextureAsset(LLGL::Texture* texture = {})
        : Asset(Type::Texture), texture(texture)
    {
        textureDesc.type = LLGL::TextureType::Texture2D;
        textureDesc.format = LLGL::Format::RGBA8UNorm;
        textureDesc.miscFlags = LLGL::MiscFlags::GenerateMips;

        imageView.format = LLGL::ImageFormat::RGBA;
        imageView.dataType = LLGL::DataType::UInt8;
    };

    GLuint nativeHandle{};

    LLGL::Texture* texture{};
    LLGL::Sampler* sampler{};

    LLGL::TextureDescriptor textureDesc;
    LLGL::ImageView imageView;
};

using TextureAssetPtr = std::shared_ptr<TextureAsset>;

}

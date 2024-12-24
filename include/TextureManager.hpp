#pragma once
#include <Utils.hpp>
#include <Multithreading.hpp>

namespace dev
{

struct TextureHandle // For multithreaded loading
{
    TextureHandle(const LLGL::TextureDescriptor& textureDesc, const LLGL::ImageView* imageView)
        : textureDesc(textureDesc)
    {
        if(imageView)
            this->imageView = *imageView;
    }

    TextureHandle(LLGL::Texture* texture = {}) : texture(texture)
    {
        textureDesc.type = LLGL::TextureType::Texture2D;
        textureDesc.format = LLGL::Format::RGBA8UNorm;
        textureDesc.miscFlags = LLGL::MiscFlags::GenerateMips;
        
        imageView.format = LLGL::ImageFormat::RGBA;
        imageView.dataType = LLGL::DataType::UInt8;
    };

    LLGL::Texture* texture{};

    LLGL::TextureDescriptor textureDesc;
    LLGL::ImageView imageView;
};

class TextureManager
{
public:
    static TextureManager& Get();

public:
    std::shared_ptr<TextureHandle> LoadTexture(const std::filesystem::path& path);

    std::shared_ptr<TextureHandle> CreateTexture(const LLGL::TextureDescriptor& textureDesc, const LLGL::ImageView* initialImage = nullptr);

    LLGL::Sampler* GetAnisotropySampler() const;

private:
    TextureManager();

private:
    LLGL::Sampler* anisotropySampler{};

    TextureHandle defaultTexture;

    std::vector<std::shared_ptr<TextureHandle>> textures;
};

}

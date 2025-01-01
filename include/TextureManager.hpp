#pragma once
#include <Utils.hpp>
#include <Multithreading.hpp>
#include <Singleton.hpp>

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

class TextureManager final : public Singleton<TextureManager>
{
public:
    std::shared_ptr<TextureHandle> LoadTexture(const std::filesystem::path& path, bool separateThread = true);

    std::shared_ptr<TextureHandle> CreateTexture(const LLGL::TextureDescriptor& textureDesc, const LLGL::ImageView* initialImage = nullptr);

    TextureHandle* GetDefaultTexture();
    LLGL::Sampler* GetAnisotropySampler() const;

private:
    TextureManager();

    friend class Singleton<TextureManager>;

private:
    LLGL::Sampler* anisotropySampler{};

    TextureHandle defaultTexture;

    std::vector<std::shared_ptr<TextureHandle>> textures;
};

}

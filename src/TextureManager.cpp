#include <TextureManager.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

TextureManager::TextureManager()
{
    LLGL::SamplerDescriptor samplerDesc;
    samplerDesc.maxAnisotropy = 8;
    anisotropySampler = Renderer::Get().CreateSampler(samplerDesc);

    LLGL::TextureDescriptor defaultTextureDesc;
    defaultTextureDesc.type = LLGL::TextureType::Texture2D;
    defaultTextureDesc.format = LLGL::Format::RGBA8UNorm;
    defaultTextureDesc.extent = { 1, 1, 1 };

    LLGL::ImageView defaultImageView;
    defaultImageView.format = LLGL::ImageFormat::RGBA;
    defaultImageView.dataType = LLGL::DataType::UInt8;
    defaultImageView.dataSize = 4 * 8;
    unsigned char defaultData[] = { 0, 0, 0, 255 };
    defaultImageView.data = defaultData;

    defaultTexture = Renderer::Get().CreateTexture(defaultTextureDesc, &defaultImageView);
}

TextureManager& TextureManager::Get()
{
    static TextureManager instance;
    
    return instance;
}

std::shared_ptr<TextureHandle> TextureManager::LoadTexture(const std::filesystem::path& path)
{
    auto textureHandle = std::make_shared<TextureHandle>(defaultTexture);

    auto textureDesc = std::make_shared<LLGL::TextureDescriptor>();
    textureDesc->type = LLGL::TextureType::Texture2D;
    textureDesc->format = LLGL::Format::RGBA8UNorm;
    textureDesc->miscFlags = LLGL::MiscFlags::GenerateMips;
    
    auto imageView = std::make_shared<LLGL::ImageView>();
    imageView->format = LLGL::ImageFormat::RGBA;
    imageView->dataType = LLGL::DataType::UInt8;

    auto load = [imageView, textureDesc, path]()
    {
        int width, height, channels;
        unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &channels, 4);

        if(data)
        {
            imageView->data = data;
            imageView->dataSize = width * height * 4 * 8;
            
            textureDesc->extent = { (uint32_t)width, (uint32_t)height, 1 };
        }
        else
            LLGL::Log::Errorf("Failed to load texture: %s\n", path.string().c_str());
    };

    auto create = [imageView, textureDesc, textureHandle]()
    {
        if(imageView->data)
            textureHandle->texture = Renderer::Get().CreateTexture(*textureDesc, imageView.get());

        stbi_image_free((void*)imageView->data);
    };

    Multithreading::Get().AddJob(load);
    Multithreading::Get().AddMainThreadJob(create);

    textures.push_back(textureHandle);

    return textureHandle;
}

LLGL::Sampler* TextureManager::GetAnisotropySampler() const
{
    return anisotropySampler;
}

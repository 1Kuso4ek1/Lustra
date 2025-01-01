#include <TextureManager.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace dev
{

TextureManager::TextureManager()
{
    LLGL::SamplerDescriptor samplerDesc;
    samplerDesc.maxAnisotropy = 8;
    anisotropySampler = Renderer::Get().CreateSampler(samplerDesc);
    
    defaultTexture.textureDesc.extent = { 1, 1, 1 };
    
    unsigned char defaultData[] = { 255, 20, 147, 255 };
    defaultTexture.imageView.dataSize = 4 * 8;
    defaultTexture.imageView.data = defaultData;

    defaultTexture = Renderer::Get().CreateTexture(defaultTexture.textureDesc, &defaultTexture.imageView);
}

/* TextureManager& TextureManager::Get()
{
    static TextureManager instance;
    
    return instance;
} */

std::shared_ptr<TextureHandle> TextureManager::LoadTexture(const std::filesystem::path& path, bool separateThread)
{
    auto textureHandle = std::make_shared<TextureHandle>(defaultTexture.texture);

    auto load = [textureHandle, path]()
    {
        int width, height, channels;
        unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &channels, 4);

        if(data)
        {
            textureHandle->imageView.data = data;
            textureHandle->imageView.dataSize = width * height * 4 * 8;
            
            textureHandle->textureDesc.extent = { (uint32_t)width, (uint32_t)height, 1 };
        }
        else
            LLGL::Log::Errorf("Failed to load texture: %s\n", path.string().c_str());
    };

    auto create = [textureHandle]()
    {
        if(textureHandle->imageView.data)
            textureHandle->texture = Renderer::Get().CreateTexture(textureHandle->textureDesc, &textureHandle->imageView);

        stbi_image_free((void*)textureHandle->imageView.data);
    };

    if(separateThread)
    {
        Multithreading::Get().AddJob(load);
        Multithreading::Get().AddMainThreadJob(create);
    }
    else
    {
        load();
        create();
    }

    textures.push_back(textureHandle);

    return textureHandle;
}

std::shared_ptr<TextureHandle> TextureManager::CreateTexture(const LLGL::TextureDescriptor& textureDesc, const LLGL::ImageView* initialImage)
{
    auto textureHandle = std::make_shared<TextureHandle>(textureDesc, initialImage);

    textureHandle->texture = Renderer::Get().CreateTexture(textureDesc, initialImage);

    textures.push_back(textureHandle);

    return textureHandle;
}

TextureHandle* TextureManager::GetDefaultTexture()
{
    return &defaultTexture;
}

LLGL::Sampler* TextureManager::GetAnisotropySampler() const
{
    return anisotropySampler;
}

}

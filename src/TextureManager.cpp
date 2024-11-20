#include <TextureManager.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace dev {

TextureManager::TextureManager()
{
    LLGL::SamplerDescriptor samplerDesc;
    samplerDesc.maxAnisotropy = 8;
    anisotropySampler = Renderer::Get().CreateSampler(samplerDesc);
    
    defaultTexture.textureDesc.extent = { 1, 1, 1 };
    
    unsigned char defaultData[] = { 0, 0, 0, 255 };
    defaultTexture.imageView.dataSize = 4 * 8;
    defaultTexture.imageView.data = defaultData;

    defaultTexture = Renderer::Get().CreateTexture(defaultTexture.textureDesc, &defaultTexture.imageView);
}

TextureManager& TextureManager::Get()
{
    static TextureManager instance;
    
    return instance;
}

std::shared_ptr<TextureHandle> TextureManager::LoadTexture(const std::filesystem::path& path)
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

    Multithreading::Get().AddJob(load);
    Multithreading::Get().AddMainThreadJob(create);

    textures.push_back(textureHandle);

    return textureHandle;
}

LLGL::Sampler* TextureManager::GetAnisotropySampler() const
{
    return anisotropySampler;
}

}

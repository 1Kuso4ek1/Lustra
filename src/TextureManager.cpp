#include <TextureManager.hpp>

TextureManager::TextureManager()
{
    LLGL::SamplerDescriptor samplerDesc;
    samplerDesc.maxAnisotropy = 8;
    anisotropySampler = Renderer::Get().CreateSampler(samplerDesc);
}

TextureManager& TextureManager::Get()
{
    static TextureManager instance;
    return instance;
}

LLGL::Texture* TextureManager::LoadTexture(const std::filesystem::path& path)
{
    return nullptr;
}

#pragma once
#include <Utils.hpp>
#include <Multithreading.hpp>

struct TextureHandle // For multithreaded loading
{
    LLGL::Texture* texture{};
};

class TextureManager
{
public:
    static TextureManager& Get();

public:
    std::shared_ptr<TextureHandle> LoadTexture(const std::filesystem::path& path);

    LLGL::Sampler* GetAnisotropySampler() const;

private:
    TextureManager();

private:
    LLGL::Sampler* anisotropySampler{};

    LLGL::Texture* defaultTexture{};

    std::vector<std::shared_ptr<TextureHandle>> textures;
};

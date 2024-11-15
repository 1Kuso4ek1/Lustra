#pragma once
#include <Utils.hpp>
#include <Multithreading.hpp>

class TextureManager
{
public:
    static TextureManager& Get();

public:
    LLGL::Texture* LoadTexture(const std::filesystem::path& path);

private:
    TextureManager();

private:
    LLGL::Sampler* anisotropySampler{};

    std::vector<LLGL::Texture*> textures;
};

#pragma once
#include <AssetLoader.hpp>
#include <TextureAsset.hpp>
#include <Renderer.hpp>
#include <Multithreading.hpp>

namespace lustra
{

class TextureLoader : public AssetLoader, public Singleton<TextureLoader>
{
public:
    AssetPtr Load(const std::filesystem::path& path) override;

private:
    void LoadDefaultData();

private:
    LLGL::Texture* defaultTexture{};
    LLGL::Texture* emptyTexture{};
    LLGL::Sampler* anisotropySampler{};

    std::shared_ptr<TextureAsset> defaultTextureAsset, emptyTextureAsset;
};

}

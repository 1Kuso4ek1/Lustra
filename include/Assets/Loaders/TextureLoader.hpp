#pragma once
#include <AssetLoader.hpp>
#include <TextureAsset.hpp>
#include <Renderer.hpp>
#include <Multithreading.hpp>

namespace dev
{

class TextureLoader : public AssetLoader, public Singleton<TextureLoader>
{
public:
    std::shared_ptr<Asset> Load(const std::filesystem::path& path) override;

private:
    void LoadDefaultData();

private:
    LLGL::Texture* defaultTexture{};
    LLGL::Sampler* anisotropySampler{};

    std::shared_ptr<TextureAsset> defaultTextureAsset;
};

}

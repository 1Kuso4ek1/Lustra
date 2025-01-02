#pragma once
#include <Asset.hpp>
#include <Singleton.hpp>

#include <LLGL/Log.h>

#include <filesystem>

namespace dev
{

class AssetLoader
{
public:
    virtual ~AssetLoader() = default;

    virtual std::shared_ptr<Asset> Load(const std::filesystem::path& path) = 0;
};

class DummyAssetLoader : public AssetLoader, public Singleton<DummyAssetLoader>
{
public:
    std::shared_ptr<Asset> Load(const std::filesystem::path& path) override
    {
        LLGL::Log::Printf(
            LLGL::Log::ColorFlags::Bold | LLGL::Log::ColorFlags::Green,
            "Loading dummy asset %s\n", path.string().c_str()
        );

        auto asset = std::make_shared<DummyAsset>();
        asset->data = path.string().size();

        return asset;
    }    
};

}

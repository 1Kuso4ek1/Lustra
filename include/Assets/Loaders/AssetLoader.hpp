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

}

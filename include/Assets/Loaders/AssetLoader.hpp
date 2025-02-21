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

    virtual AssetPtr Load(const std::filesystem::path& path) = 0;
    virtual void Write(const AssetPtr& asset, const std::filesystem::path& path) {};
};

}

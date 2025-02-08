#pragma once
#include <Asset.hpp>

#include <filesystem>

namespace dev
{

struct ScriptAsset : public Asset
{
    ScriptAsset(std::filesystem::path path)
        : Asset(Type::Script), path(path) {}

    std::filesystem::path path;
};

using ScriptAssetPtr = std::shared_ptr<ScriptAsset>;

}

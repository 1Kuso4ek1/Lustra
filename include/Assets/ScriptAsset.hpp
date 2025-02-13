#pragma once
#include <Asset.hpp>

namespace dev
{

struct ScriptAsset : public Asset
{
    ScriptAsset() : Asset(Type::Script) {}

    uint32_t modulesCount = 0;
};

using ScriptAssetPtr = std::shared_ptr<ScriptAsset>;

}

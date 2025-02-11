#pragma once
#include <Asset.hpp>

namespace dev
{

struct ScriptAsset : public Asset
{
    ScriptAsset() : Asset(Type::Script) {}
};

using ScriptAssetPtr = std::shared_ptr<ScriptAsset>;

}

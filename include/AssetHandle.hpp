#pragma once
#include <memory>

namespace dev
{

template<class T>
class AssetHandle
{
public:
    using Asset = std::shared_ptr<T>;

    AssetHandle(Asset asset, std::string name = "") : asset(asset), name(name) {};

    Asset GetAsset() const { return asset; }

    std::string GetName() const { return name; }

    operator bool() const { return asset != nullptr; }

private:
    Asset asset;

    std::string name;
};

}

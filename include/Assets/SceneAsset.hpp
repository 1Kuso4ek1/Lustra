#pragma once
#include <Scene.hpp>

namespace dev
{

struct SceneAsset : public Asset
{
    SceneAsset() : Asset(Type::Scene) {}

    std::shared_ptr<Scene> scene;
};

using SceneAssetPtr = std::shared_ptr<SceneAsset>;

}

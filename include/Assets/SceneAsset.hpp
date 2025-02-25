#pragma once
#include <Scene.hpp>

namespace lustra
{

struct SceneAsset : public Asset
{
    SceneAsset(std::shared_ptr<Scene> scene)
        : Asset(Type::Scene), scene(scene) {}

    std::shared_ptr<Scene> scene;
};

using SceneAssetPtr = std::shared_ptr<SceneAsset>;

}

#pragma once
#include <Scene.hpp>

namespace lustra
{

struct SceneAsset final : Asset
{
    explicit SceneAsset(const std::shared_ptr<Scene>& scene)
        : Asset(Type::Scene), scene(scene) {}

    std::shared_ptr<Scene> scene;
};

using SceneAssetPtr = std::shared_ptr<SceneAsset>;

}

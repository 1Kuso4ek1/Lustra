#pragma once
#include <Asset.hpp>

#include <LLGL/Texture.h>

namespace lustra
{

struct EnvironmentAsset : public Asset
{
    EnvironmentAsset() : Asset(Type::Environment) {}

    EnvironmentAsset(
        LLGL::Texture* cubeMap,
        LLGL::Texture* irradiance,
        LLGL::Texture* prefiltered,
        LLGL::Texture* brdf
    ) : Asset(Type::Environment), 
        cubeMap(cubeMap),
        irradiance(irradiance),
        prefiltered(prefiltered),
        brdf(brdf) {}

    LLGL::Texture* cubeMap{};
    LLGL::Texture* irradiance{};
    LLGL::Texture* prefiltered{};
    LLGL::Texture* brdf{};
};

using EnvironmentAssetPtr = std::shared_ptr<EnvironmentAsset>;

}

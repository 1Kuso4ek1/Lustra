#pragma once
#include <entt/entt.hpp>

#include <PhysicsManager.hpp>
#include <Mesh.hpp>
#include <ImGuiManager.hpp>
#include <Camera.hpp>
#include <PostProcessing.hpp>
#include <PBRManager.hpp>
#include <AssetManager.hpp>
#include <TextureAsset.hpp>
#include <MaterialAsset.hpp>
#include <ModelAsset.hpp>
#include <ScriptAsset.hpp>
#include <ShaderAsset.hpp>
#include <SoundAsset.hpp>

namespace lustra
{

class Entity;

struct ComponentBase
{
    explicit ComponentBase(const std::string_view& componentName) : componentName(componentName) {}
    virtual ~ComponentBase() = default;

    std::string_view componentName;
};

}

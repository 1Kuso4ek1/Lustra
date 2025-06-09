#pragma once
#include <JoltInclude.hpp>

namespace lustra
{

namespace Layers
{
    static constexpr JPH::ObjectLayer nonMoving = 0;
    static constexpr JPH::ObjectLayer moving = 1;
    static constexpr JPH::ObjectLayer numLayers = 2;
}

namespace BroadPhaseLayers
{
	static constexpr JPH::BroadPhaseLayer nonMoving(0);
	static constexpr JPH::BroadPhaseLayer moving(1);
	static constexpr uint32_t numLayers(2);
}

}

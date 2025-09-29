#pragma once
#include <Layers.hpp>

namespace lustra
{

class BroadPhaseLayerInterface final : public JPH::BroadPhaseLayerInterface
{
public:
    BroadPhaseLayerInterface()
	{
		mObjectToBroadPhase[Layers::nonMoving] = BroadPhaseLayers::nonMoving;
		mObjectToBroadPhase[Layers::moving] = BroadPhaseLayers::moving;
	}

    uint32_t GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::numLayers;
	}

    JPH::BroadPhaseLayer GetBroadPhaseLayer(const JPH::ObjectLayer inLayer) const override
	{
		return mObjectToBroadPhase[inLayer];
	}

    const char* GetBroadPhaseLayerName(const JPH::BroadPhaseLayer inLayer) const override
	{
		switch (static_cast<JPH::BroadPhaseLayer::Type>(inLayer))
		{
		case static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::nonMoving): return "Non moving";
		case static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::moving): return "Moving";

		default: return "Invalid";
		}
	}

private:
	JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::numLayers]{};
};

}

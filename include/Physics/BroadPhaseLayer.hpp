#pragma once
#include <Layers.hpp>

namespace dev
{

class BroadPhaseLayerInterface : public JPH::BroadPhaseLayerInterface
{
public:
    BroadPhaseLayerInterface()
	{
		mObjectToBroadPhase[Layers::nonMoving] = BroadPhaseLayers::nonMoving;
		mObjectToBroadPhase[Layers::moving] = BroadPhaseLayers::moving;
	}

	virtual uint32_t GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::numLayers;
	}

	virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
	{
		return mObjectToBroadPhase[inLayer];
	}

    virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
	{
		switch ((JPH::BroadPhaseLayer::Type)inLayer)
		{
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::nonMoving: return "Non moving";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::moving: return "Moving";

		default: return "Invalid";
		}
	}

private:
	JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::numLayers];
};

}

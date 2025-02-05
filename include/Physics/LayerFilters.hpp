#pragma once
#include <Layers.hpp>

namespace dev
{

class ObjectLayerPairFilter : public JPH::ObjectLayerPairFilter
{
public:
	virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
	{
		switch (inObject1)
		{
		case Layers::nonMoving: return inObject2 == Layers::moving;
		case Layers::moving: return true;

		default: return false;
		}
	}
};

class ObjectVsBroadPhaseLayerFilter : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
	{
		switch (inLayer1)
		{
		case Layers::nonMoving: return inLayer2 == BroadPhaseLayers::moving;
		case Layers::moving: return true;
        
		default: return false;
		}
	}
};

}

#pragma once
#include <Layers.hpp>

namespace lustra
{

class ObjectLayerPairFilter final : public JPH::ObjectLayerPairFilter
{
public:
	bool ShouldCollide(const JPH::ObjectLayer inObject1, const JPH::ObjectLayer inObject2) const override
	{
		switch (inObject1)
		{
		case Layers::nonMoving: return inObject2 == Layers::moving;
		case Layers::moving: return true;

		default: return false;
		}
	}
};

class ObjectVsBroadPhaseLayerFilter final : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	bool ShouldCollide(const JPH::ObjectLayer inLayer1, const JPH::BroadPhaseLayer inLayer2) const override
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

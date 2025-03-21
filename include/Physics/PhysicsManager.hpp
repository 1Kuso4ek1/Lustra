#pragma once
#include <Singleton.hpp>

#include <LayerFilters.hpp>
#include <BroadPhaseLayer.hpp>
#include <CollisionListener.hpp>

namespace lustra
{

class PhysicsManager : public Singleton<PhysicsManager>
{
public:
    ~PhysicsManager();

    void Init();
    void Update(float deltaTime);
    void DestroyBody(const JPH::BodyID& bodyId);

    JPH::Shape* CreateBoxShape(const JPH::BoxShapeSettings& settings);

    JPH::Body* CreateBody(const JPH::BodyCreationSettings& settings);

    JPH::PhysicsSystem& GetPhysicsSystem();
    JPH::BodyInterface& GetBodyInterface();

private:
    PhysicsManager();

    friend class Singleton<PhysicsManager>;

private:
    std::unique_ptr<CollisionListener> collisionListener;
    std::unique_ptr<JPH::JobSystemThreadPool> jobSystem;
    std::unique_ptr<JPH::TempAllocatorImpl> tempAllocator;

    std::unique_ptr<JPH::PhysicsSystem> physicsSystem;

    BroadPhaseLayerInterface broadPhaseLayer{};
	ObjectVsBroadPhaseLayerFilter objectVsBroadPhaseFilter{};
	ObjectLayerPairFilter objectLayerPairFilter{};
};

}

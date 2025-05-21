#pragma once
#include <Singleton.hpp>

#include <BroadPhaseLayer.hpp>
#include <CollisionListener.hpp>
#include <LayerFilters.hpp>

namespace lustra
{

class PhysicsManager final : public Singleton<PhysicsManager>
{
public:
    ~PhysicsManager() override;

    void Init();
    void Update(float deltaTime) const;
    void DestroyBody(const JPH::BodyID& bodyId) const;

    static JPH::Shape* CreateBoxShape(const JPH::BoxShapeSettings& settings);

    JPH::Body* CreateBody(const JPH::BodyCreationSettings& settings) const;

    JPH::PhysicsSystem& GetPhysicsSystem() const;
    JPH::BodyInterface& GetBodyInterface() const;

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

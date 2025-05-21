#include <PhysicsManager.hpp>

namespace lustra
{

PhysicsManager::PhysicsManager()
{
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();
}

PhysicsManager::~PhysicsManager()
{
    JPH::UnregisterTypes();

    delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;
}

void PhysicsManager::Init()
{
    physicsSystem = std::make_unique<JPH::PhysicsSystem>();

    collisionListener = std::make_unique<CollisionListener>();

    tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);

    jobSystem = std::make_unique<JPH::JobSystemThreadPool>(
        JPH::cMaxPhysicsJobs,
        JPH::cMaxPhysicsBarriers,
        std::thread::hardware_concurrency() - 1
    );

    physicsSystem->Init(
        1024,
        0,
        1024,
        1024,
        broadPhaseLayer,
        objectVsBroadPhaseFilter,
        objectLayerPairFilter
    );

    physicsSystem->SetContactListener(collisionListener.get());
}

void PhysicsManager::Update(const float deltaTime) const
{
    physicsSystem->OptimizeBroadPhase();
    physicsSystem->Update(deltaTime, 1, tempAllocator.get(), jobSystem.get());
}

void PhysicsManager::DestroyBody(const JPH::BodyID& bodyId) const
{
    GetBodyInterface().RemoveBody(bodyId);
    GetBodyInterface().DestroyBody(bodyId);
}

JPH::Shape* PhysicsManager::CreateBoxShape(const JPH::BoxShapeSettings& settings)
{
    return settings.Create().Get();
}

JPH::Body* PhysicsManager::CreateBody(const JPH::BodyCreationSettings& settings) const
{
    const auto body = GetBodyInterface().CreateBody(settings);

    GetBodyInterface().AddBody(body->GetID(), JPH::EActivation::Activate);

    return body;
}

JPH::PhysicsSystem& PhysicsManager::GetPhysicsSystem() const
{
    return *physicsSystem;
}

JPH::BodyInterface& PhysicsManager::GetBodyInterface() const
{
    return physicsSystem->GetBodyInterface();
}

}

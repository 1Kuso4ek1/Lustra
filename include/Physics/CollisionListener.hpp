#pragma once
#include <JoltInclude.hpp>
#include <EventManager.hpp>
#include <Multithreading.hpp>

#include <glm/vec3.hpp>

namespace lustra
{

class CollisionEvent : public Event
{
public:
    CollisionEvent(JPH::Body* body1, JPH::Body* body2, const JPH::ContactManifold& manifold)
        : Event(Type::Collision), body1(body1), body2(body2)
    {
        JPH::RVec3 point1, point2;
        
        if(manifold.mRelativeContactPointsOn1.size() > 0)
            point1 = manifold.GetWorldSpaceContactPointOn1(0);
        if(manifold.mRelativeContactPointsOn2.size() > 0)
            point2 = manifold.GetWorldSpaceContactPointOn2(0);

        auto normal = manifold.mWorldSpaceNormal;

        contactWorldPosition1 = glm::vec3(point1.GetX(), point1.GetY(), point1.GetZ());
        contactWorldPosition2 = glm::vec3(point2.GetX(), point2.GetY(), point2.GetZ());

        worldNormal = glm::vec3(normal.GetX(), normal.GetY(), normal.GetZ());

        penetrationDepth = manifold.mPenetrationDepth;
    }

    JPH::Body* GetBody1() const { return body1; }
    JPH::Body* GetBody2() const { return body2; }

    glm::vec3 GetContactPosition1() const { return contactWorldPosition1; }
    glm::vec3 GetContactPosition2() const { return contactWorldPosition2; }

    glm::vec3 GetNormal() const { return worldNormal; }

    float GetPenetration() const { return penetrationDepth; }

private:
    JPH::Body* body1;
    JPH::Body* body2;

    glm::vec3 contactWorldPosition1;
    glm::vec3 contactWorldPosition2;

    glm::vec3 worldNormal;

    float penetrationDepth;
};

class CollisionListener : public JPH::ContactListener
{
public:
    virtual JPH::ValidateResult OnContactValidate(
        const JPH::Body& body1,
        const JPH::Body& body2,
        JPH::RVec3Arg baseOffset,
        const JPH::CollideShapeResult& result
    ) override
    {
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    virtual void OnContactAdded(
        const JPH::Body& body1,
        const JPH::Body& body2,
        const JPH::ContactManifold& manifold,
        JPH::ContactSettings& settings
    ) override
    {
        ProcessCollision(body1, body2, manifold);
    }

private:
    void ProcessCollision(
        const JPH::Body& body1,
        const JPH::Body& body2, 
        const JPH::ContactManifold& manifold
    )
    {
        Multithreading::Get().AddJob(
            {
                nullptr,
                [&]() {
                    EventManager::Get().Dispatch(
                        std::make_unique<CollisionEvent>(
                            const_cast<JPH::Body*>(&body1),
                            const_cast<JPH::Body*>(&body2),
                            manifold
                        )
                    );
                }
            }
        );
    }

};

};

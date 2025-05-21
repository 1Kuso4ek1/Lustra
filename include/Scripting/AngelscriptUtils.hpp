#pragma once
#include <scriptstdstring.h>
#include <scriptarray.h>
#include <aswrappedcall.h>
#include <scriptmath.h>
#include <datetime.h>
#include <scriptdictionary.h>
#include <scriptfile.h>

#include <LLGL/Log.h>

#include <Scene.hpp>
#include <Entity.hpp>
#include <SceneAsset.hpp>
#include <ScriptManager.hpp>
#include <Random.hpp>

namespace lustra::as
{

struct RayCastResult
{
    bool hit = false;

    glm::vec3 hitPosition{ 0.0f };

    JPH::Body* body{};
};

inline void MessageCallback(const asSMessageInfo *msg, void *param)
{
    switch(msg->type)
    {
    case asMSGTYPE_ERROR:
        LLGL::Log::Printf(
            LLGL::Log::ColorFlags::StdError,
            "%s (%d, %d): %s\n",
            msg->section, msg->row, msg->col, msg->message
        );
        break;

    case asMSGTYPE_WARNING:
        LLGL::Log::Printf(
            LLGL::Log::ColorFlags::StdWarning,
            "%s (%d, %d): %s\n",
            msg->section, msg->row, msg->col, msg->message
        );
        break;

    case asMSGTYPE_INFORMATION:
        LLGL::Log::Printf(
            LLGL::Log::ColorFlags::Blue,
            "%s (%d, %d): %s\n",
            msg->section, msg->row, msg->col, msg->message
        );
        break;
    }
}

template<class T>
inline void MakeType(T* memory) { new(memory) T(); }

template<class T>
inline void CopyType(const T& other, T* memory) { new(memory) T(other); }

template<class T>
inline void DestroyType(T* memory) { memory->~T(); }

template<class T>
inline T& AssignType(const T& src, T* dst) { return *dst = src; }

template<class T>
inline T* TypeFactory() { return new T(); }

inline void Write(const std::string& data)
{
    LLGL::Log::Printf("%s", data.c_str());
}

inline Camera* GetCamera(CameraComponent* comp)
{
    return &(comp->camera);
}

inline void MakeExtent2D(const uint32_t width, const uint32_t height, LLGL::Extent2D* extent)
{
    new(extent) LLGL::Extent2D(width, height);
}

inline void MakeVec2(const float x, const float y, glm::vec2* vec)
{
    new(vec) glm::vec2(x, y);
}

inline void MakeVec2Scalar(const float scalar, glm::vec2* vec)
{
    new(vec) glm::vec2(scalar);
}

inline void MakeVec3(const float x, const float y, const float z, glm::vec3* vec)
{
    new(vec) glm::vec3(x, y, z);
}

inline void MakeVec3Scalar(const float scalar, glm::vec3* vec)
{
    new(vec) glm::vec3(scalar);
}

inline void MakeVec4(const float x, const float y, const float z, const float w, glm::vec4* vec)
{
    new(vec) glm::vec4(x, y, z, w);
}

inline void MakeVec4Scalar(const float scalar, glm::vec4* vec)
{
    new(vec) glm::vec4(scalar);
}

inline void MakeQuat(const float x, const float y, const float z, const float w, glm::quat* quat)
{
    new(quat) glm::quat(w, x, y, z);
}

inline void MakeQuatFromEuler(const glm::vec3& euler, glm::quat* quat)
{
    new(quat) glm::quat(euler);
}

inline void MakeQuatFromMat4(const glm::mat4& mat, glm::quat* quat)
{
    new(quat) glm::quat(mat);
}

inline glm::vec2 Vec2AddAssign(const glm::vec2& src, glm::vec2& dst)
{
    return dst += src;
}

inline glm::vec2 Vec2SubAssign(const glm::vec2& src, glm::vec2& dst)
{
    return dst -= src;
}

inline glm::vec2 Vec2MulAssign(const glm::vec2& src, glm::vec2& dst)
{
    return dst *= src;
}

inline glm::vec2 Vec2DivAssign(const glm::vec2& src, glm::vec2& dst)
{
    return dst /= src;
}

inline glm::vec2 Vec2MulAssignScalar(const float src, glm::vec2* dst)
{
    return *dst *= src;
}

inline glm::vec2 Vec2DivAssignScalar(const float src, glm::vec2* dst)
{
    return *dst /= src;
}

inline glm::vec3 Vec3AddAssign(const glm::vec3& src, glm::vec3* dst)
{
    return *dst += src;
}

inline glm::vec3 Vec3SubAssign(const glm::vec3& src, glm::vec3* dst)
{
    return *dst -= src;
}

inline glm::vec3 Vec3MulAssign(const glm::vec3& src, glm::vec3* dst)
{
    return *dst *= src;
}

inline glm::vec3 Vec3DivAssign(const glm::vec3& src, glm::vec3* dst)
{
    return *dst /= src;
}

inline glm::vec3 Vec3MulAssignScalar(const float src, glm::vec3* dst)
{
    return *dst *= src;
}

inline glm::vec3 Vec3DivAssignScalar(const float src, glm::vec3* dst)
{
    return *dst /= src;
}

inline glm::vec4 Vec4AddAssign(const glm::vec4& src, glm::vec4* dst)
{
    return *dst += src;
}

inline glm::vec4 Vec4SubAssign(const glm::vec4& src, glm::vec4* dst)
{
    return *dst -= src;
}

inline glm::vec4 Vec4MulAssign(const glm::vec4& src, glm::vec4* dst)
{
    return *dst *= src;
}

inline glm::vec4 Vec4DivAssign(const glm::vec4& src, glm::vec4* dst)
{
    return *dst /= src;
}

inline glm::vec4 Vec4MulAssignScalar(const float src, glm::vec4* dst)
{
    return *dst *= src;
}

inline glm::vec4 Vec4DivAssignScalar(const float src, glm::vec4* dst)
{
    return *dst /= src;
}

inline glm::vec3 GetPosition(const JPH::Body* body)
{
    const auto pos = body->GetPosition();

    return { pos.GetX(), pos.GetY(), pos.GetZ() };
}

inline glm::quat GetRotation(const JPH::Body* body)
{
    const auto rot = body->GetRotation();

    return { rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ() };
}

inline void SetPosition(const glm::vec3& pos, const JPH::Body* body)
{
    PhysicsManager::Get().GetBodyInterface().SetPosition(body->GetID(), { pos.x, pos.y, pos.z }, JPH::EActivation::Activate);
}

inline void SetRotation(const glm::quat& rot, const JPH::Body* body)
{
    PhysicsManager::Get().GetBodyInterface().SetRotation(body->GetID(), { rot.x, rot.y, rot.z, rot.w }, JPH::EActivation::Activate);
}

inline void SetPositionAndRotation(const glm::vec3& pos, const glm::quat& rot, const JPH::Body* body)
{
    PhysicsManager::Get().GetBodyInterface().SetPositionAndRotation(body->GetID(), { pos.x, pos.y, pos.z }, { rot.x, rot.y, rot.z, rot.w }, JPH::EActivation::Activate);
}

inline void AddForce(const glm::vec3& force, const JPH::Body* body)
{
    PhysicsManager::Get().GetBodyInterface().AddForce(body->GetID(), { force.x, force.y, force.z });
}

inline void AddTorque(const glm::vec3& torque, const JPH::Body* body)
{
    PhysicsManager::Get().GetBodyInterface().AddTorque(body->GetID(), { torque.x, torque.y, torque.z });
}

inline void AddImpulse(const glm::vec3& impulse, const JPH::Body* body)
{
    PhysicsManager::Get().GetBodyInterface().AddImpulse(body->GetID(), { impulse.x, impulse.y, impulse.z });
}

inline void SetLinearVelocity(const glm::vec3& velocity, const JPH::Body* body)
{
    PhysicsManager::Get().GetBodyInterface().SetLinearVelocity(body->GetID(), { velocity.x, velocity.y, velocity.z });
}

inline void SetAngularVelocity(const glm::vec3& velocity, const JPH::Body* body)
{
    PhysicsManager::Get().GetBodyInterface().SetAngularVelocity(body->GetID(), { velocity.x, velocity.y, velocity.z });
}

inline glm::vec3 GetLinearVelocity(const JPH::Body* body)
{
    const auto vel = body->GetLinearVelocity();

    return { vel.GetX(), vel.GetY(), vel.GetZ() };
}

inline glm::vec3 GetAngularVelocity(const JPH::Body* body)
{
    const auto vel = body->GetAngularVelocity();

    return { vel.GetX(), vel.GetY(), vel.GetZ() };
}

inline glm::mat4 GetWorldTransform(const Entity& entity, Scene* scene)
{
    return scene->GetWorldTransform(entity);
}

inline RayCastResult CastRay(const glm::vec3& origin, const glm::vec3& direction)
{
    JPH::RayCastResult result;
    RayCastResult ret;

    JPH::RRayCast ray;
    ray.mOrigin = { origin.x, origin.y, origin.z };
    ray.mDirection = { direction.x, direction.y, direction.z };

    if(PhysicsManager::Get().GetPhysicsSystem().GetNarrowPhaseQuery().CastRay(ray, result))
    {
        ret.hit = true;
        ret.hitPosition = origin + direction * result.mFraction;
        ret.body = PhysicsManager::Get().GetPhysicsSystem().GetBodyLockInterface().TryGetBody(result.mBodyID);
    }

    return ret;
}

inline void MapAction(const std::string& action, const Keyboard::Key key)
{
    InputManager::Get().MapAction(action, key);
}

inline void MapAction(const std::string& action, const Mouse::Button button)
{
    InputManager::Get().MapAction(action, button);
}

inline bool IsActionPressed(const std::string& action)
{
    return InputManager::Get().IsActionPressed(action);
}

template<class T>
std::shared_ptr<T> Load(
    const std::string& path,
    const bool relativeToAssetsDir = false,
    const bool useCache = true,
    const bool async = true
)
{
    return AssetManager::Get().Load<T>(path, relativeToAssetsDir, useCache, async);
}

template<class T>
T* GetAssetPtr(std::shared_ptr<T>* asset)
{
    return asset->get();
}

inline MaterialAssetPtr& MaterialListAt(const uint64_t index, MeshRendererComponent* component)
{
    return component->materials[index];
}

inline void ExecuteFunction(const ScriptAssetPtr& script, const std::string& declaration, const uint32_t moduleIndex)
{
    Multithreading::Get().AddJob(
        { nullptr, [&]() { ScriptManager::Get().ExecuteFunction(script, declaration, nullptr, moduleIndex); } }
    );
}

inline void RandomSetSeed(const uint32_t seed)
{
    Random::Get().SetSeed(seed);
}

inline float RandomValue()
{
    return Random::Get().Value();
}

inline float RandomRange(const float min, const float max)
{
    return Random::Get().Range(min, max);
}

inline int RandomRange(const int min, const int max)
{
    return Random::Get().Range(min, max);
}

inline void SetGravityFactor(const float factor, const JPH::Body* body)
{
    PhysicsManager::Get().GetBodyInterface().SetGravityFactor(body->GetID(), factor);
}

inline void SetMass(const float mass, JPH::Body* body)
{
    JPH::MassProperties newProperties;
    newProperties.mMass = mass;
    newProperties.mInertia = JPH::Mat44::sIdentity();

    const auto properties = body->GetMotionProperties();
    properties->SetMassProperties(JPH::EAllowedDOFs::All, newProperties);
}

}

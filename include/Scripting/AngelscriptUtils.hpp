#pragma once
#include <angelscript.h>

#include <scriptbuilder.h>
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

namespace dev
{

namespace as
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

inline void MakeExtent2D(uint32_t width, uint32_t height, LLGL::Extent2D* extent)
{
    new(extent) LLGL::Extent2D(width, height);
}

inline void MakeVec2(float x, float y, glm::vec2* vec)
{
    new(vec) glm::vec2(x, y);
}

inline void MakeVec2Scalar(float scalar, glm::vec2* vec)
{
    new(vec) glm::vec2(scalar);
}

inline void MakeVec3(float x, float y, float z, glm::vec3* vec)
{
    new(vec) glm::vec3(x, y, z);
}

inline void MakeVec3Scalar(float scalar, glm::vec3* vec)
{
    new(vec) glm::vec3(scalar);
}

inline void MakeVec4(float x, float y, float z, float w, glm::vec4* vec)
{
    new(vec) glm::vec4(x, y, z, w);
}

inline void MakeVec4Scalar(float scalar, glm::vec4* vec)
{
    new(vec) glm::vec4(scalar);
}

inline void MakeQuat(float x, float y, float z, float w, glm::quat* quat)
{
    new(quat) glm::quat(x, y, z, w);
}

inline void MakeQuatFromEuler(const glm::vec3& euler, glm::quat* quat)
{
    new(quat) glm::quat(euler);
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

inline glm::vec2 Vec2MulAssignScalar(float src, glm::vec2* dst)
{
    return *dst *= src;
}

inline glm::vec2 Vec2DivAssignScalar(float src, glm::vec2* dst)
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

inline glm::vec3 Vec3MulAssignScalar(float src, glm::vec3* dst)
{
    return *dst *= src;
}

inline glm::vec3 Vec3DivAssignScalar(float src, glm::vec3* dst)
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

inline glm::vec4 Vec4MulAssignScalar(float src, glm::vec4* dst)
{
    return *dst *= src;
}

inline glm::vec4 Vec4DivAssignScalar(float src, glm::vec4* dst)
{
    return *dst /= src;
}

inline glm::vec3 GetPosition(JPH::Body* body)
{
    auto pos = body->GetPosition();
    
    return { pos.GetX(), pos.GetY(), pos.GetZ() };
}

inline glm::quat GetRotation(JPH::Body* body)
{
    auto rot = body->GetRotation();

    return { rot.GetX(), rot.GetY(), rot.GetZ(), rot.GetW() };
}

inline void SetPosition(const glm::vec3& pos, JPH::Body* body)
{
    PhysicsManager::Get().GetBodyInterface().SetPosition(body->GetID(), { pos.x, pos.y, pos.z }, JPH::EActivation::Activate);
}

inline void SetRotation(const glm::quat& rot, JPH::Body* body)
{
    PhysicsManager::Get().GetBodyInterface().SetRotation(body->GetID(), { rot.x, rot.y, rot.z, rot.w }, JPH::EActivation::Activate);
}

inline void SetPositionAndRotation(const glm::vec3& pos, const glm::quat& rot, JPH::Body* body)
{
    PhysicsManager::Get().GetBodyInterface().SetPositionAndRotation(body->GetID(), { pos.x, pos.y, pos.z }, { rot.x, rot.y, rot.z, rot.w }, JPH::EActivation::Activate);
}

inline void AddForce(const glm::vec3& force, JPH::Body* body)
{
    PhysicsManager::Get().GetBodyInterface().AddForce(body->GetID(), { force.x, force.y, force.z });
}

inline void AddTorque(const glm::vec3& torque, JPH::Body* body)
{
    PhysicsManager::Get().GetBodyInterface().AddTorque(body->GetID(), { torque.x, torque.y, torque.z });
}

inline void AddImpulse(const glm::vec3& impulse, JPH::Body* body)
{
    PhysicsManager::Get().GetBodyInterface().AddImpulse(body->GetID(), { impulse.x, impulse.y, impulse.z });
}

inline void SetLinearVelocity(const glm::vec3& velocity, JPH::Body* body)
{
    PhysicsManager::Get().GetBodyInterface().SetLinearVelocity(body->GetID(), { velocity.x, velocity.y, velocity.z });
}

inline void SetAngularVelocity(const glm::vec3& velocity, JPH::Body* body)
{
    PhysicsManager::Get().GetBodyInterface().SetAngularVelocity(body->GetID(), { velocity.x, velocity.y, velocity.z });
}

inline glm::vec3 GetLinearVelocity(JPH::Body* body)
{
    auto vel = body->GetLinearVelocity();
    
    return { vel.GetX(), vel.GetY(), vel.GetZ() };
}

inline glm::vec3 GetAngularVelocity(JPH::Body* body)
{
    auto vel = body->GetAngularVelocity();

    return { vel.GetX(), vel.GetY(), vel.GetZ() };
}

inline glm::mat4 GetWorldTransform(Entity entity, Scene* scene)
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

inline void MapAction(const std::string& action, Keyboard::Key key)
{
    InputManager::Get().MapAction(action, key);
}

inline void MapAction(const std::string& action, Mouse::Button button)
{
    InputManager::Get().MapAction(action, button);
}

inline bool IsActionPressed(const std::string& action)
{
    return InputManager::Get().IsActionPressed(action);
}

inline TextureAssetPtr LoadTexture(const std::string& path, bool relativeToAssetsDir = false)
{
    return AssetManager::Get().Load<TextureAsset>(path, relativeToAssetsDir);
}

inline MaterialAssetPtr LoadMaterial(const std::string& path, bool relativeToAssetsDir = false)
{
    return AssetManager::Get().Load<MaterialAsset>(path, relativeToAssetsDir);
}

inline ModelAssetPtr LoadModel(const std::string& path, bool relativeToAssetsDir = false)
{
    return AssetManager::Get().Load<ModelAsset>(path, relativeToAssetsDir);
}

template<class T>
inline T* GetAssetPtr(std::shared_ptr<T>* asset)
{
    return asset->get();
}

}

}

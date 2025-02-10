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

namespace dev
{

namespace as
{

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

}

}

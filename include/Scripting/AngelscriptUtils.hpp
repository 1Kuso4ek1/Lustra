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
            "%s (%s, %s): %s",
            msg->section, msg->row, msg->col, msg->message
        );
        break;

    case asMSGTYPE_WARNING:
        LLGL::Log::Printf(
            LLGL::Log::ColorFlags::StdWarning,
            "%s (%s, %s): %s",
            msg->section, msg->row, msg->col, msg->message
        );
        break;

    case asMSGTYPE_INFORMATION:
        LLGL::Log::Printf(
            LLGL::Log::ColorFlags::Blue,
            "%s (%s, %s): %s",
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

}

}

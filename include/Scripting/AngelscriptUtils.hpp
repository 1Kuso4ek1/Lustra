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

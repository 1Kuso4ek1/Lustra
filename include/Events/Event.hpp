#pragma once
#include <LLGL/Types.h>

namespace lustra
{

class Event
{
public:
    enum class Type
    {
        Undefined,
        WindowResize,
        WindowFocus,
        AssetLoaded,
        Collision
    };

    virtual ~Event() = default;

    void SetHandled() { handled = true; }

    Type GetType() const { return type; }

    bool IsHandled() const { return handled; }

protected:
    explicit Event(const Type type) : type(type) {}

private:
    Type type = Event::Type::Undefined;

    bool handled = false;
};

}

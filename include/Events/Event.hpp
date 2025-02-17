#pragma once
#include <LLGL/Types.h>

namespace dev
{

class Event
{
public:
    enum class Type
    {
        Undefined, WindowResize, AssetLoaded, Collision
    };

    virtual ~Event() = default;

    void SetHandled() { handled = true; }

    Type GetType() const { return type; }

    bool IsHandled() const { return handled; }

protected:
    Event(Type type) : type(type) {}

private:
    Type type = Event::Type::Undefined;

    bool handled = false;
};

}

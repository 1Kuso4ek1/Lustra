#pragma once
#include <LLGL/Types.h>

namespace dev
{

class Event
{
public:
    enum class Type
    {
        None, WindowResize
    };

    virtual ~Event() = default;

    void SetHandled() { handled = true; }

    Type GetType() const { return type; }

    bool IsHandled() const { return handled; }

protected:
    Event(Type type) : type(type) {}

private:
    Type type = Type::None;

    bool handled = false;
};

class WindowResizeEvent : public Event
{
public:
    WindowResizeEvent(const LLGL::Extent2D& size) : Event(Type::WindowResize), size(size) {}

    LLGL::Extent2D GetSize() const { return size; }

private:
    LLGL::Extent2D size;
};

}

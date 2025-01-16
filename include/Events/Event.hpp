#pragma once
#include <Asset.hpp>

#include <LLGL/Types.h>

namespace dev
{

class Event
{
public:
    enum class Type
    {
        Undefined, WindowResize, AssetLoaded
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

class WindowResizeEvent : public Event
{
public:
    WindowResizeEvent(const LLGL::Extent2D& size) : Event(Type::WindowResize), size(size) {}

    LLGL::Extent2D GetSize() const { return size; }

private:
    LLGL::Extent2D size;
};

class AssetLoadedEvent : public Event
{
public:
    AssetLoadedEvent(AssetPtr asset) : Event(Type::AssetLoaded), asset(asset) {}

    AssetPtr GetAsset() const { return asset; }

private:
    AssetPtr asset;
};

}

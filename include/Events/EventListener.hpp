#pragma once
#include <Event.hpp>

namespace lustra
{

class EventListener
{
public:
    virtual ~EventListener() = default;

    virtual void OnEvent(Event& event) = 0;
};

}

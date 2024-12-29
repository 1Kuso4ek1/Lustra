#pragma once
#include <Event.hpp>

namespace dev
{

class EventListener
{
public:
    virtual ~EventListener() = default;

    virtual void OnEvent(Event& event) = 0;
};

}

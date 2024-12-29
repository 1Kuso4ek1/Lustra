#pragma once
#include <EventListener.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

namespace dev
{

class EventManager
{
public:
    static EventManager& Get();

public:
    void AddListener(Event::Type eventType, EventListener* listener);
    void RemoveListener(Event::Type eventType, EventListener* listener);

    void Dispatch(std::unique_ptr<Event> event);

private:
    EventManager() {};

private:
    std::unordered_map<Event::Type, std::vector<EventListener*>> listeners;
};

}

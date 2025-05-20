#include <EventManager.hpp>

#include <algorithm>

namespace lustra
{

EventManager::~EventManager()
{
    listeners.clear();
}

void EventManager::AddListener(const Event::Type eventType, EventListener* listener)
{
    auto& vec = listeners[eventType];
    if(std::ranges::find(vec, listener) != vec.end())
        return;

    vec.push_back(listener);
}

void EventManager::RemoveListener(const Event::Type eventType, EventListener* listener)
{
    if(listeners.empty())
        return;

    auto& vec = listeners[eventType];
    std::erase(vec, listener);
}

void EventManager::Dispatch(const std::unique_ptr<Event>& event)
{
    const auto& vec = listeners[event->GetType()];

    for(const auto listener : vec)
    {
        listener->OnEvent(*event);

        if(event->IsHandled())
            break;
    }
}

}

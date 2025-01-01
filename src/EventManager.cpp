#include <EventManager.hpp>
#include <algorithm>

namespace dev
{

void EventManager::AddListener(Event::Type eventType, EventListener* listener)
{
    listeners[eventType].push_back(listener);
}

void EventManager::RemoveListener(Event::Type eventType, EventListener* listener)
{
    auto& vec = listeners[eventType];
    vec.erase(std::remove(vec.begin(), vec.end(), listener), vec.end());
}

void EventManager::Dispatch(std::unique_ptr<Event> event)
{
    auto& vec = listeners[event->GetType()];

    for(auto listener : vec)
    {
        listener->OnEvent(*event);
        
        if(event->IsHandled())
            break;
    }
}

}

#pragma once
#include <EventListener.hpp>
#include <Singleton.hpp>

#include <memory>
#include <unordered_map>
#include <vector>

namespace lustra
{

class EventManager final : public Singleton<EventManager>
{
public:
    ~EventManager() override;

    void AddListener(Event::Type eventType, EventListener* listener);
    void RemoveListener(Event::Type eventType, EventListener* listener);

    void Dispatch(const std::unique_ptr<Event>& event);

private:
    std::unordered_map<Event::Type, std::vector<EventListener*>> listeners;
};

}

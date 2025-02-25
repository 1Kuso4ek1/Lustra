#pragma once
#include <Singleton.hpp>
#include <Keyboard.hpp>
#include <Mouse.hpp>

#include <unordered_map>

namespace lustra
{

class InputManager : public Singleton<InputManager>
{
public:
    void Update();

    void MapAction(const std::string& action, Keyboard::Key key);
    void MapAction(const std::string& action, Mouse::Button button);

    bool IsActionPressed(const std::string& action);

private:
    std::unordered_map<std::string, std::vector<Keyboard::Key>> keyboardMapping;
    std::unordered_map<std::string, std::vector<Mouse::Button>> mouseMapping;

    std::unordered_map<std::string, bool> actionStates;
};

}

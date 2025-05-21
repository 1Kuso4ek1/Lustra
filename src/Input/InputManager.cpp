#include <InputManager.hpp>

namespace lustra
{

void InputManager::Update()
{
    for(auto const& [action, keys] : keyboardMapping)
    {
        bool isPressed = false;

        for(auto& key : keys)
            if(Keyboard::IsKeyPressed(key))
            {
                isPressed = true;
                break;
            }

        actionStates[action] = isPressed;
    }

    for(auto const& [action, buttons] : mouseMapping)
    {
        bool isPressed = false;

        for(auto& button : buttons)
            if(Mouse::IsButtonPressed(button))
            {
                isPressed = true;
                break;
            }

        actionStates[action] = isPressed;
    }
}

void InputManager::MapAction(const std::string& action, const Keyboard::Key key)
{
    keyboardMapping[action].push_back(key);
}

void InputManager::MapAction(const std::string& action, const Mouse::Button button)
{
    mouseMapping[action].push_back(button);
}

bool InputManager::IsActionPressed(const std::string& action)
{
    const auto it = actionStates.find(action);

    return it == actionStates.end() ? false : it->second;
}

}

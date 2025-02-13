#include <InputManager.hpp>

namespace dev
{

void InputManager::Update()
{
    for(auto [action, keys] : keyboardMapping)
    {
        bool isPressed = false;

        for(auto key : keys)
            if(Keyboard::IsKeyPressed(key))
            {
                isPressed = true;
                break;
            }

        actionStates[action] = isPressed;
    }

    for(auto [action, buttons] : mouseMapping)
    {
        bool isPressed = false;

        for(auto button : buttons)
            if(Mouse::IsButtonPressed(button))
            {
                isPressed = true;
                break;
            }

        actionStates[action] = isPressed;
    }
}

void InputManager::MapAction(const std::string& action, Keyboard::Key key)
{
    keyboardMapping[action].push_back(key);
}

void InputManager::MapAction(const std::string& action, Mouse::Button button)
{
    mouseMapping[action].push_back(button);
}

bool InputManager::IsActionPressed(const std::string& action)
{
    auto it = actionStates.find(action);

    return it == actionStates.end() ? false : it->second;
}

}

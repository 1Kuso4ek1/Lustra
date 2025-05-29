#pragma once
#include <Window.hpp>

#include <glm/fwd.hpp>
#include <glm/vec2.hpp>

namespace lustra::Mouse
{

struct Scroll
{
    static glm::vec2 offset;
};

enum class Button
{
    Left = GLFW_MOUSE_BUTTON_LEFT,
    Right = GLFW_MOUSE_BUTTON_RIGHT,
    Middle = GLFW_MOUSE_BUTTON_MIDDLE,
    MButton4 = GLFW_MOUSE_BUTTON_4,
    MButton5 = GLFW_MOUSE_BUTTON_5,
    MButton6 = GLFW_MOUSE_BUTTON_6,
    MButton7 = GLFW_MOUSE_BUTTON_7,
    MButton8 = GLFW_MOUSE_BUTTON_8,
    Last = GLFW_MOUSE_BUTTON_LAST
};

void ScrollCallback(GLFWwindow* window, double x, double y);

void SetCursorVisible(bool visible = true);

void SetPosition(const glm::vec2& pos);

bool IsButtonPressed(Button button);
bool IsButtonReleased(Button button);
bool IsButtonRepeated(Button button);

glm::vec2 GetPosition();

}

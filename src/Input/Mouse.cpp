#include <Mouse.hpp>

namespace dev
{

namespace Mouse
{

void SetCursorVisible(bool visible)
{
    glfwSetInputMode(Window::GetLastCreatedGLFWWindow(), GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void SetPosition(const glm::vec2& pos)
{
    glfwSetCursorPos(Window::GetLastCreatedGLFWWindow(), pos.x, pos.y);
}

bool IsButtonPressed(Button button)
{
    return glfwGetMouseButton(Window::GetLastCreatedGLFWWindow(), static_cast<int>(button)) == GLFW_PRESS;
}

bool IsButtonReleased(Button button)
{
    return glfwGetMouseButton(Window::GetLastCreatedGLFWWindow(), static_cast<int>(button)) == GLFW_RELEASE;
}

bool IsButtonRepeated(Button button)
{
    return glfwGetMouseButton(Window::GetLastCreatedGLFWWindow(), static_cast<int>(button)) == GLFW_REPEAT;
}

glm::vec2 GetPosition()
{
    double x, y;
    glfwGetCursorPos(Window::GetLastCreatedGLFWWindow(), &x, &y);

    return { x, y };
}

}

}

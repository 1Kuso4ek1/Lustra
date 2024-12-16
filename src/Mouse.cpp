#include <Mouse.hpp>

namespace dev
{

namespace Mouse
{

bool IsButtonPressed(Button button)
{
    return glfwGetMouseButton(dev::Window::GetLastCreatedGLFWWindow(), static_cast<int>(button)) == GLFW_PRESS;
}

bool IsButtonReleased(Button button)
{
    return glfwGetMouseButton(dev::Window::GetLastCreatedGLFWWindow(), static_cast<int>(button)) == GLFW_RELEASE;
}

bool IsButtonRepeated(Button button)
{
    return glfwGetMouseButton(dev::Window::GetLastCreatedGLFWWindow(), static_cast<int>(button)) == GLFW_REPEAT;
}

glm::vec2 GetPosition()
{
    double x, y;
    glfwGetCursorPos(dev::Window::GetLastCreatedGLFWWindow(), &x, &y);

    return { x, y };
}

}

}

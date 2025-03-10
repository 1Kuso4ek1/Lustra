#include <Keyboard.hpp>

namespace lustra
{

namespace Keyboard
{

bool IsKeyPressed(Key key)
{
    return glfwGetKey(Window::GetLastCreatedGLFWWindow(), static_cast<int>(key)) == GLFW_PRESS;
}

bool IsKeyReleased(Key key)
{
    return glfwGetKey(Window::GetLastCreatedGLFWWindow(), static_cast<int>(key)) == GLFW_RELEASE;
}

bool IsKeyRepeated(Key key)
{
    return glfwGetKey(Window::GetLastCreatedGLFWWindow(), static_cast<int>(key)) == GLFW_REPEAT;
}

}

}

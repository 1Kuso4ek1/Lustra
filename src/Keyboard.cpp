#include <Keyboard.hpp>

namespace dev {

namespace Keyboard {

bool IsKeyPressed(Key key)
{
    return glfwGetKey(dev::Window::GetLastCreatedGLFWWindow(), static_cast<int>(key)) == GLFW_PRESS;
}

bool IsKeyReleased(Key key)
{
    return glfwGetKey(dev::Window::GetLastCreatedGLFWWindow(), static_cast<int>(key)) == GLFW_RELEASE;
}

bool IsKeyRepeated(Key key)
{
    return glfwGetKey(dev::Window::GetLastCreatedGLFWWindow(), static_cast<int>(key)) == GLFW_REPEAT;
}

}

}

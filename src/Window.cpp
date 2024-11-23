#include <Window.hpp>

namespace dev {

bool Window::glfwInitialized = false;
GLFWwindow* Window::lastCreatedWindow{};

Window::Window(const LLGL::Extent2D& size, const std::string_view& title, bool fullscreen)
    : size(size), title(title), fullscreen(fullscreen)
{
    if(!glfwInitialized)
    {
        #ifdef __linux__
            glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11); // So we don't use wayland
        #endif

        glfwInit();

        glfwInitialized = true;
    }

    lastCreatedWindow = window = CreateWindow();
}

Window::~Window()
{
    glfwDestroyWindow(window);
}

void Window::SwapBuffers()
{
    glfwSwapBuffers(window);
}

GLFWwindow* Window::GetGLFWWindow() const
{
    return window;
}

GLFWwindow* Window::GetLastCreatedGLFWWindow()
{
    return lastCreatedWindow;
}

bool Window::PollEvents() const
{
    glfwPollEvents();

    return !glfwWindowShouldClose(window);
}

bool Window::GetNativeHandle(void* nativeHandle, size_t size)
{
    if(nativeHandle && size == sizeof(LLGL::NativeHandle) && window)
    {
        auto nativeHandlePtr = reinterpret_cast<LLGL::NativeHandle*>(nativeHandle);

        #ifdef _WIN32
            nativeHandlePtr->window = glfwGetWin32Window(window);
        #elif __APPLE__
            nativeHandlePtr->responder = glfwGetCocoaWindow(window);
        #elif __linux__
            nativeHandlePtr->window = glfwGetX11Window(window);
            nativeHandlePtr->display = glfwGetX11Display();
        #endif

        return true;
    }

    return false;
}

bool Window::AdaptForVideoMode(LLGL::Extent2D* resolution, bool* fullscreen)
{
    glfwSetWindowSize(window, resolution->width, resolution->height);

    return true;
}

LLGL::Extent2D Window::GetContentSize() const
{
    int x, y;
    glfwGetWindowSize(window, &x, &y);

    return { (uint32_t)x, (uint32_t)y };
}

LLGL::Display* Window::FindResidentDisplay() const
{
    return LLGL::Display::GetPrimary();
}

GLFWwindow* Window::CreateWindow()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    auto window = glfwCreateWindow(size.width, size.height, title.data(), fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

    if(!window)
        throw std::runtime_error("Failed to create GLFW window");

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    return window;
}

}

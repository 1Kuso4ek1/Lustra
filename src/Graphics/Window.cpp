#include <Window.hpp>

namespace lustra
{

bool Window::glfwInitialized = false;
GLFWwindow* Window::lastCreatedWindow{};

static void OnWindowResize(GLFWwindow* window, const int width, const int height)
{
    auto event = std::make_unique<WindowResizeEvent>(LLGL::Extent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) });

    EventManager::Get().Dispatch(std::move(event));
}

static void OnWindowFocus(GLFWwindow* window, const int focused)
{
    auto event = std::make_unique<WindowFocusEvent>(static_cast<bool>(focused));

    EventManager::Get().Dispatch(std::move(event));
}

Window::Window(const LLGL::Extent2D& size, const std::string_view& title, const int samples, const bool fullscreen)
    : samples(samples), fullscreen(fullscreen), size(size), title(title)
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

    glfwTerminate();

    glfwInitialized = false;

    if(lastCreatedWindow == window)
        lastCreatedWindow = nullptr;
}

void Window::Close() const
{
    glfwSetWindowShouldClose(window, true);
}

void Window::SetFullscreen(const bool fullscreen)
{
    this->fullscreen = fullscreen;

    glfwSetWindowMonitor(window, fullscreen ? glfwGetPrimaryMonitor() : nullptr, 0, 0,
        static_cast<int>(size.width), static_cast<int>(size.height), GLFW_DONT_CARE);
}

void Window::Maximize() const
{
    glfwMaximizeWindow(window);
}

void Window::Minimize() const
{
    glfwIconifyWindow(window);
}

void Window::Restore() const
{
    glfwRestoreWindow(window);
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

bool Window::IsFullscreen() const
{
    return fullscreen;
}

bool Window::GetNativeHandle(void* nativeHandle, const size_t size)
{
    if(nativeHandle && size == sizeof(LLGL::NativeHandle) && window)
    {
        auto nativeHandlePtr = static_cast<LLGL::NativeHandle*>(nativeHandle);

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
    glfwSetWindowSize(window, static_cast<int>(resolution->width), static_cast<int>(resolution->height));

    return true;
}

LLGL::Extent2D Window::GetContentSize() const
{
    int x, y;
    glfwGetWindowSize(window, &x, &y);

    return { static_cast<uint32_t>(x), static_cast<uint32_t>(y) };
}

LLGL::Display* Window::FindResidentDisplay() const
{
    return LLGL::Display::GetPrimary();
}

GLFWwindow* Window::CreateWindow() const
{
    const auto window =
        glfwCreateWindow(static_cast<int>(size.width), static_cast<int>(size.height),
            title.data(), fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

    if(!window)
        throw std::runtime_error("Failed to create GLFW window");

    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetFramebufferSizeCallback(window, OnWindowResize);
    glfwSetWindowFocusCallback(window, OnWindowFocus);

    return window;
}

}

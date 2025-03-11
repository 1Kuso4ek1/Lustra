#include <Window.hpp>

namespace lustra
{

bool Window::glfwInitialized = false;
GLFWwindow* Window::lastCreatedWindow{};

static void OnWindowResize(GLFWwindow* window, int width, int height)
{
    auto event = std::make_unique<WindowResizeEvent>(LLGL::Extent2D{ (uint32_t)width, (uint32_t)height });

    EventManager::Get().Dispatch(std::move(event));
}

static void OnWindowFocus(GLFWwindow* window, int focused)
{
    auto event = std::make_unique<WindowFocusEvent>((bool)focused);

    EventManager::Get().Dispatch(std::move(event));
}

Window::Window(const LLGL::Extent2D& size, const std::string_view& title, int samples, bool fullscreen)
    : size(size), title(title), samples(samples), fullscreen(fullscreen)
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

void Window::Close()
{
    glfwSetWindowShouldClose(window, true);
}

void Window::SetFullscreen(bool fullscreen)
{
    this->fullscreen = fullscreen;

    glfwSetWindowMonitor(window, fullscreen ? glfwGetPrimaryMonitor() : nullptr, 0, 0, size.width, size.height, GLFW_DONT_CARE);
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
    auto window = glfwCreateWindow(size.width, size.height, title.data(), fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

    if(!window)
        throw std::runtime_error("Failed to create GLFW window");

    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetFramebufferSizeCallback(window, OnWindowResize);
    glfwSetWindowFocusCallback(window, OnWindowFocus);

    return window;
}

}

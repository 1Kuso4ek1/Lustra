#pragma once

#ifdef _WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif __APPLE__
    #define GLFW_EXPOSE_NATIVE_COCOA
#elif __linux__
    #define GLFW_EXPOSE_NATIVE_X11
#endif

#include <EventManager.hpp>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <LLGL/Surface.h>
#include <LLGL/Platform/NativeHandle.h>

namespace dev
{

static void OnWindowResize(GLFWwindow* window, int width, int height);

class Window : public LLGL::Surface
{
public:
    Window(const LLGL::Extent2D& size, const std::string_view& title, int samples = 1, bool fullscreen = false);
    ~Window();

    void SetFullscreen(bool fullscreen);

    GLFWwindow* GetGLFWWindow() const;
    static GLFWwindow* GetLastCreatedGLFWWindow();

    bool PollEvents() const;
    bool IsFullscreen() const;

public: // Interface implementation
    bool GetNativeHandle(void* nativeHandle, size_t size) override;
    bool AdaptForVideoMode(LLGL::Extent2D* resolution, bool* fullscreen) override;
    
    LLGL::Extent2D GetContentSize() const override;
    LLGL::Display* FindResidentDisplay() const override;
    
private:
    GLFWwindow* CreateWindow();

private:
    static bool glfwInitialized;
    static GLFWwindow* lastCreatedWindow;

    int samples = 1;

    bool fullscreen = false;

    LLGL::Extent2D size;
    std::string_view title;

    GLFWwindow* window{};
};

class WindowResizeEvent : public Event
{
public:
    WindowResizeEvent(const LLGL::Extent2D& size) : Event(Type::WindowResize), size(size) {}

    LLGL::Extent2D GetSize() const { return size; }

private:
    LLGL::Extent2D size;
};

using WindowPtr = std::shared_ptr<Window>;

// There are some #include combinations where these X11 defines interfere with other libraries
#ifdef __linux__
    #undef Convex
    #undef None
#endif

}

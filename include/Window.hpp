#pragma once

#ifdef _WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif __APPLE__
    #define GLFW_EXPOSE_NATIVE_COCOA
#elif __linux__
    #define GLFW_EXPOSE_NATIVE_X11
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <LLGL/Surface.h>
#include <LLGL/Platform/NativeHandle.h>

namespace glfw
{

class Window : public LLGL::Surface
{
public:
    Window(const LLGL::Extent2D& size, const std::string_view& title);
    ~Window();

    void SwapBuffers();

    bool PollEvents();

public: // Interface implementation
    void ResetPixelFormat() override;

    bool GetNativeHandle(void* nativeHandle, size_t size) override;
    bool AdaptForVideoMode(LLGL::Extent2D* resolution, bool* fullscreen) override;
    
    LLGL::Extent2D GetContentSize() const override;
    LLGL::Display* FindResidentDisplay() const override;
    
private:
    GLFWwindow* CreateWindow();

private:
    static bool glfwInitialized;

    LLGL::Extent2D size;
    std::string_view title;

    GLFWwindow* window{};
};

using WindowPtr = std::shared_ptr<Window>;

}

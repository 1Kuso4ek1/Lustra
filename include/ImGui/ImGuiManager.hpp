#pragma once
#include <Singleton.hpp>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <misc/cpp/imgui_stdlib.h>

#include <filesystem>

namespace lustra
{

class ImGuiManager : public Singleton<ImGuiManager>
{
public:
    void Init(
        GLFWwindow* window,
        const std::filesystem::path& fontPath = {},
        const std::filesystem::path& iniPath = {}
    );
    void NewFrame();
    void Render();
    void Destroy();

private:
    void SetupStyle();

private:
    bool initialized = false;

    std::string iniPath;
};

}

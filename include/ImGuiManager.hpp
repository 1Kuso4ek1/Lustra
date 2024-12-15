#pragma once
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <misc/cpp/imgui_stdlib.h>

#include <filesystem>

namespace dev
{

class ImGuiManager
{
public:
    static ImGuiManager& Get();

public:
    void Init(GLFWwindow* window, std::filesystem::path fontPath = {});
    void NewFrame();
    void Render();
    void Destroy();

private:
    ImGuiManager() = default;

private:
    void SetupStyle();
};

}

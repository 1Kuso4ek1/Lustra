#pragma once
#include <Singleton.hpp>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <misc/cpp/imgui_stdlib.h>

#include <filesystem>

namespace dev
{

class ImGuiManager : public Singleton<ImGuiManager>
{
public:
    void Init(GLFWwindow* window, std::filesystem::path fontPath = {});
    void NewFrame();
    void Render();
    void Destroy();

private:
    void SetupStyle();
};

}

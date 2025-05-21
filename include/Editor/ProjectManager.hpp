#pragma once
#include <Application.hpp>

class ProjectManager final : public lustra::Application
{
public:
    ProjectManager();
    ~ProjectManager() override;

    void Stop() override;

    void Init() override;
    void Update(float deltaTime) override;
    void Render() override;
    void RenderImGui() override;

public:
    static bool IsProjectOpened();

private:
    void CreateProject(const std::filesystem::path& path);

private:
    std::vector<std::filesystem::path> recentProjects;

    const std::vector<std::filesystem::path> projectStructure =
    {
        "config", "fonts", "materials",
        "models", "scenes", "scripts",
        "shaders", "sounds", "textures"
    };

    const std::vector<std::filesystem::path> copy =
    {
        "fonts/OpenSans-Regular.ttf",
        "materials/default",
        "models/cube",
        "models/plane",
        "textures/default",
        "textures/empty"
    };

private:
    bool showCreatePopup = false;
    bool showOpenPopup = false;

    static bool projectOpened;

    std::string newProjectName, newProjectPath;

private:
    lustra::TextureAssetPtr logo;
};

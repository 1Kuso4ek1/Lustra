#include <Editor.hpp>
#include <ProjectManager.hpp>

int main()
{
    std::filesystem::current_path(EDITOR_ROOT);

    ProjectManager().Run();
    
    if(ProjectManager::IsProjectOpened())
    {
        Editor(
            lustra::Config::Load(
                std::filesystem::current_path().string() == EDITOR_ROOT
                    ? "resources/config/config.json"
                    : "assets/config/config.json"
            )
        ).Run();
    }
}

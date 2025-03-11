#include <Editor.hpp>
#include <ProjectManager.hpp>

int main()
{
    std::filesystem::current_path(EDITOR_ROOT);

    ProjectManager().Run();
    Editor(
        lustra::Config::Load("resources/config/config.json")
    ).Run();
}

#include <Editor.hpp>
#include <ProjectManager.hpp>

int main()
{
    ProjectManager().Run();
    Editor(
        lustra::Config::Load("../resources/config/config.json")
    ).Run();
}

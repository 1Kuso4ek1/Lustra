#include <Launcher.hpp>

int main()
{
    Launcher launcher(lustra::Config::Load("config.json"));

    launcher.Run();
}

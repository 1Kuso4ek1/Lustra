#include <Launcher.hpp>

int main()
{
    Launcher launcher(lustra::Config::Load("assets/config/config.json"));

    launcher.Run();
}

#include <Editor.hpp>

int main()
{
    Editor app(lustra::Config::Load("../resources/config/config.json"));

    app.Run();
}

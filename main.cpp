#include <Editor.hpp>

int main()
{
    Editor app(dev::Config::Load("../resources/config/config.json"));

    app.Run();
}

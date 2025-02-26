#include <Application.hpp>

class Launcher : public lustra::Application
{
public:
    Launcher(const lustra::Config& config);

    void Init() override;
    void Update(float deltaTime) override;
    void Render() override;

private:

};

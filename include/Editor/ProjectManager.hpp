#include <Application.hpp>

class ProjectManager : public lustra::Application
{
public:
    ProjectManager();
    ~ProjectManager();

    void Init() override;
    void Update(float deltaTime) override;
    void Render() override;

private:
    void RenderImGui();

private:
    lustra::TextureAssetPtr logo;
};

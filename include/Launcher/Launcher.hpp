#include <Application.hpp>

class Launcher : public lustra::Application, public lustra::EventListener
{
public:
    Launcher(const lustra::Config& config);

    void Init() override;
    void Update(float deltaTime) override;
    void Render() override;
    //void RenderImGui() override;

    void OnEvent(lustra::Event& event) override;

private:
    lustra::SceneAssetPtr sceneAsset;
    std::shared_ptr<lustra::Scene> scene;
};

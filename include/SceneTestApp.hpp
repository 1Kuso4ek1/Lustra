#pragma once
#include <Application.hpp>
#include <TextureManager.hpp>

#include <Scene.hpp>
#include <Entity.hpp>

#include <Window.hpp>

#include <Keyboard.hpp>
#include <Mouse.hpp>

#include <ImGuiManager.hpp>

class SceneTestApp : public dev::Application
{
public:
    SceneTestApp();
    ~SceneTestApp();

    void Run() override;

private:
    void LoadShaders();
    void LoadTextures();

    void CreateEntities();

    void CreateCubeEntity();
    void CreateCameraEntity();
    void CreatePostProcessingEntity();
    void CreateLightEntity();
    void CreateLight1Entity();
    
    void DrawImGui();
    void Draw();

    float degrees = 0.0f;

    glm::vec3 axis{ 0.0f, 1.0f, 0.0f };
    float angle = 0.0f;

    dev::Entity entity, camera, postProcessing, light, light1;

    dev::Scene scene;

    dev::WindowPtr window;

    dev::Timer deltaTimeTimer, keyboardTimer;

    std::shared_ptr<dev::Mesh> mesh;

    LLGL::PipelineState* pipeline{};

    LLGL::Shader* vertexShader{};
    LLGL::Shader* fragmentShader{};

    std::shared_ptr<dev::TextureHandle> texture;

    std::shared_ptr<dev::Matrices> matrices;
};

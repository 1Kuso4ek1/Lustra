#include <ProjectManager.hpp>

ProjectManager::ProjectManager()
    : lustra::Application(lustra::Config{
        .resolution = { 1280, 720 },
        .vsync = true,
        .title = "Lustra project manager"
    })
{
    Init();
}

ProjectManager::~ProjectManager()
{
    lustra::AssetManager::Get().RemoveLoader<lustra::TextureAsset>();
}

void ProjectManager::Init()
{
    lustra::AssetManager::Get().AddLoader<lustra::TextureAsset, lustra::TextureLoader>("textures");

    logo = lustra::AssetManager::Get().Load<lustra::TextureAsset>("../resources/branding/logo.png");
}

void ProjectManager::Update(float deltaTime)
{
    
}

void ProjectManager::Render()
{
    lustra::Renderer::Get().ClearRenderTarget();

    RenderImGui();

    lustra::Renderer::Get().Present();
}

void ProjectManager::RenderImGui()
{
    lustra::ImGuiManager::Get().NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("Project Manager", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

    ImGui::Image(logo->nativeHandle, { 1024, 418 });
    
    ImGui::End();
    ImGui::PopStyleVar();

    lustra::ImGuiManager::Get().Render();
}

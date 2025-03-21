#include <ProjectManager.hpp>

namespace fs = std::filesystem;

ProjectManager::ProjectManager()
    : lustra::Application(lustra::Config{
        .resolution = { 1280, 720 },
        .vsync = true,
        .title = "Lustra project manager",
        .imGuiFontPath = "resources/fonts/OpenSans-Regular.ttf",
        .imGuiLayoutPath = "null"
    })
{
    Init();
}

ProjectManager::~ProjectManager()
{
    lustra::AssetManager::Get().RemoveLoader<lustra::TextureAsset>();

    lustra::Renderer::Get().Release(logo->texture);
}

void ProjectManager::Init()
{
    lustra::AssetManager::Get().LaunchWatch();
    lustra::AssetManager::Get().AddLoader<lustra::TextureAsset, lustra::TextureLoader>("");

    logo = lustra::AssetManager::Get().Load<lustra::TextureAsset>("resources/branding/logo.png");
}

void ProjectManager::Update(float deltaTime) {}

void ProjectManager::Render() {}

void ProjectManager::RenderImGui()
{
    lustra::Renderer::Get().ClearRenderTarget();
    
    lustra::ImGuiManager::Get().NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("Project Manager", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 600) * 0.5f);

    ImGui::Image(logo->nativeHandle, { 512, 214 });

    ImGui::Separator();

    if(ImGui::Button("New Project"))
    {
        showCreatePopup = true;
        newProjectName.clear();
        newProjectPath = getenv("HOME");
    }
    
    if(ImGui::Button("Open Project"))
    {
        showOpenPopup = true;
        newProjectName.clear();
        newProjectPath = getenv("HOME");
    }
    
    if(showCreatePopup)
    {
        ImGui::OpenPopup("Create New Project");
        
        if(ImGui::BeginPopupModal("Create New Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::InputText("Project Name", &newProjectName);
            ImGui::InputText("Project Path", &newProjectPath);

            ImGui::SameLine();

            if(ImGui::Button("Browse...")) { /* use https://github.com/samhocevar/portable-file-dialogs.git? */ }
            
            ImGui::TextDisabled("(a folder \"%s\" will be created in this directory)", newProjectName.c_str());

            bool canCreate = !newProjectName.empty() && !newProjectPath.empty() && fs::exists(newProjectPath);
            if(!canCreate)
                ImGui::BeginDisabled();
            
            if(ImGui::Button("Create"))
            {
                try
                {
                    CreateProject(fs::path(newProjectPath) / newProjectName);

                    fs::current_path(fs::path(newProjectPath) / newProjectName);

                    Stop();
                }
                catch(const std::exception& e)
                {
                    LLGL::Log::Errorf(
                        LLGL::Log::ColorFlags::StdError,
                        "%s", e.what()
                    );
                }

                showCreatePopup = false;
                ImGui::CloseCurrentPopup();
            }
            
            if(!canCreate)
                ImGui::EndDisabled();
            
            ImGui::SameLine();
            if(ImGui::Button("Cancel"))
            {
                showCreatePopup = false;
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
    }

    if(showOpenPopup)
    {
        ImGui::OpenPopup("Open Project");
        
        if(ImGui::BeginPopupModal("Open Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::InputText("Project Path", &newProjectPath);

            ImGui::SameLine();

            if(ImGui::Button("Browse...")) { /* use https://github.com/samhocevar/portable-file-dialogs.git? */ }

            bool canOpen = !newProjectPath.empty() && fs::exists(newProjectPath);
            if(!canOpen)
                ImGui::BeginDisabled();

            if(ImGui::Button("Open"))
            {
                fs::current_path(newProjectPath);

                Stop();
            }

            if(!canOpen)
                ImGui::EndDisabled();

            ImGui::EndPopup();
        }
    }
    
    ImGui::End();
    ImGui::PopStyleVar();

    lustra::ImGuiManager::Get().Render();
}

void ProjectManager::CreateProject(const fs::path& path)
{
    fs::create_directory(path);
    fs::create_directory(path / "assets");

    for(auto& folder : projectStructure)
        fs::create_directory(path / "assets" / folder);

    for(auto& file : copy)
        fs::copy_file(
            EDITOR_ROOT / ("resources" / file),
            path / "assets" / file,
            fs::copy_options::overwrite_existing
        );
    
    for(auto& shader : fs::directory_iterator(EDITOR_ROOT / fs::path("resources/shaders")))
    {
        if(shader.is_regular_file())
            fs::copy_file(
                shader.path(),
                path / "assets" / "shaders" / shader.path().filename(),
                fs::copy_options::overwrite_existing
            );
    }

    lustra::Config{
        .resolution = { 1280, 720 },
        .vsync = false,
        .title = newProjectName,
        .imGuiFontPath = EDITOR_ROOT"/resources/fonts/OpenSans-Regular.ttf",
        .imGuiLayoutPath = EDITOR_ROOT"/resources/layout/editor_layout.ini"
    }.Save(path / "assets/config/config.json");
}

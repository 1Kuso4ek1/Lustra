#include <ProjectManager.hpp>

namespace fs = std::filesystem;

bool ProjectManager::projectOpened = false;

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

    std::ofstream file(EDITOR_ROOT"/resources/config/recent.json");

    if(file.is_open())
    {
        cereal::JSONOutputArchive archive(file);

        archive(cereal::make_nvp("size",recentProjects.size()));

        for(auto& project : recentProjects)
            archive(cereal::make_nvp("project", project.string()));
    }
}

void ProjectManager::Stop()
{
    projectOpened = true;

    lustra::Application::Stop();
}

void ProjectManager::Init()
{
    lustra::AssetManager::Get().LaunchWatch();
    lustra::AssetManager::Get().AddLoader<lustra::TextureAsset, lustra::TextureLoader>("");

    logo = lustra::AssetManager::Get().Load<lustra::TextureAsset>("resources/branding/logo.png");

    std::ifstream file(EDITOR_ROOT"/resources/config/recent.json");

    if(file.is_open())
    {
        cereal::JSONInputArchive archive(file);

        int size;
        std::string in;

        archive(size);

        for(int i = 0; i < size; i++)
        {
            archive(in);
            recentProjects.emplace_back(in);
        }
    }
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

    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 512) * 0.5f);
    ImGui::Image(logo->nativeHandle, { 512, 214 });

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::BeginGroup();

    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 200.0f) * 0.5f);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.4f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.6f));
    
    if(ImGui::Button("New Project", ImVec2(200.0f, 40.0f))) {
        showCreatePopup = true;
        newProjectName.clear();
        newProjectPath = getenv("HOME");
    }
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 200.0f) * 0.5f);
    if(ImGui::Button("Open Project", ImVec2(200.0f, 40.0f))) {
        showOpenPopup = true;
        newProjectName.clear();
        newProjectPath = getenv("HOME");
    }

    ImGui::PopStyleColor(2);

    ImGui::EndGroup();

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
                    auto path = fs::path(newProjectPath) / newProjectName;

                    CreateProject(path);

                    fs::current_path(path);

                    auto it = std::find(recentProjects.begin(), recentProjects.end(), path);
                    if(it != recentProjects.end())
                        recentProjects.erase(it);

                    recentProjects.insert(recentProjects.begin(), fs::path(path));

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

                auto it = std::find(recentProjects.begin(), recentProjects.end(), newProjectPath);
                if(it != recentProjects.end())
                    recentProjects.erase(it);

                recentProjects.insert(recentProjects.begin(), fs::path(newProjectPath));

                Stop();
            }

            if(!canOpen)
                ImGui::EndDisabled();

            ImGui::SameLine();

            if(ImGui::Button("Cancel"))
            {
                showOpenPopup = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Recent Projects").x) * 0.5f);
    ImGui::Text("Recent Projects");

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    const ImVec4 evenColor = ImVec4(0.13f, 0.13f, 0.13f, 1.0f);
    const ImVec4 oddColor = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);
    const ImVec4 hoverColor = ImVec4(0.5, 0.5f, 0.5f, 0.5f);

    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 500.0f) * 0.5f);
    ImGui::BeginChild("Recent projects", { 500.0f, 200 }, ImGuiChildFlags_FrameStyle);

    for(int i = 0; i < recentProjects.size(); i++)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, (i % 2) ? oddColor : evenColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, hoverColor);

        if(
            ImGui::Button(
                recentProjects[i].filename().string().c_str(),
                { ImGui::GetContentRegionAvail().x, 30 }
            )
        )
        {
            fs::current_path(recentProjects[i]);

            if(recentProjects.size() > 1)
            {
                recentProjects.insert(recentProjects.begin(), recentProjects[i]);
                recentProjects.erase(recentProjects.begin() + i + 1);
            }

            Stop();

            ImGui::PopStyleColor(3);

            break;
        }

        if(ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("%s", recentProjects[i].string().c_str());
            ImGui::EndTooltip();
        }

        ImGui::PopStyleColor(3);
    }

    if(recentProjects.empty())
        ImGui::TextDisabled("No recent projects yet");

    ImGui::EndChild();

    ImGui::PopStyleVar();

    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 60);
    ImGui::Separator();

    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 40);
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("GitHub").x) * 0.5f);

    ImGui::TextLinkOpenURL("GitHub", "https://github.com/1Kuso4ek1/Lustra");

    ImGui::End();
    ImGui::PopStyleVar();

    lustra::ImGuiManager::Get().Render();
}

bool ProjectManager::IsProjectOpened()
{
    return projectOpened;
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

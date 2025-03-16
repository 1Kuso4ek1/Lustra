#include <Editor.hpp>

void Editor::DrawMaterialPreview(lustra::MaterialAssetPtr material, const ImVec2& size)
{
    if(material->albedo.type == lustra::MaterialAsset::Property::Type::Texture)
    {
        if(ImGui::ImageButton("##Asset", material->albedo.texture->nativeHandle, size))
            selectedAsset = material;
    }
    else
    {
        ImVec4 color = ImVec4(material->albedo.value.x, material->albedo.value.y, material->albedo.value.z, material->albedo.value.w);
        
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
        
        if(ImGui::Button("##Asset", size))
            selectedAsset = material;

        ImGui::PopStyleColor(3); 
    }
}

// Kinda messy, clean it up
void Editor::DrawAssetBrowser()
{
    static const auto assetsPath = lustra::AssetManager::Get().GetAssetsDirectory();
    auto selectedAssetPath = assetsPath;

    static auto currentDirectory = assetsPath;
    static std::string filter;
    
    static const auto& assets = lustra::AssetManager::Get().GetAssets();

    ImGui::Begin("Assets");

    float regionWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
    int cols = std::max(1, (int)(regionWidth / 128.0f));

    ImGui::Text("%s", currentDirectory.c_str());

    ImGui::SameLine();
    
    if(ImGui::Button("..") && currentDirectory.has_parent_path())
        currentDirectory = currentDirectory.parent_path();

    ImGui::Separator();

    ImGui::InputText("Filter", &filter);

    ImGui::Separator();

    ImGui::Columns(cols, nullptr, false);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

    for(const auto& entry : std::filesystem::directory_iterator(currentDirectory))
    {
        ImGui::PushID(entry.path().c_str());

        if(entry.is_directory())
        {
            if(filter.empty() || entry.path().filename().string().find(filter) != std::string::npos)
            {
                if(ImGui::ImageButton("##Directory", folderIcon->nativeHandle, ImVec2(128.0f, 128.0f)))
                    currentDirectory = entry.path();

                ImGui::Text("%s", entry.path().filename().string().c_str());

                ImGui::NextColumn();
            }
        }
        else
        {
            if(filter.empty() || entry.path().filename().string().find(filter) != std::string::npos)
            {
                auto it = assets.find(entry.path());
                if(it != assets.end())
                    DrawAsset(entry.path(), it->second.second);
                else
                    DrawUnloadedAsset(entry.path());

                ImGui::Text("%s", entry.path().filename().string().c_str());

                ImGui::NextColumn();
            }
        }

        ImGui::PopID();
    }

    ImGui::Columns();

    ImGui::PopStyleVar();

    DrawCreateAssetMenu(currentDirectory);

    ImGui::End();
}

void Editor::DrawAsset(const std::filesystem::path& entry, lustra::AssetPtr asset)
{
    switch(asset->type)
    {
        case lustra::Asset::Type::Texture:
        {
            auto texture = std::dynamic_pointer_cast<lustra::TextureAsset>(asset);
            
            ImGui::ImageButton("##Asset", texture->nativeHandle, ImVec2(128.0f, 128.0f));

            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                lustra::TextureAssetPtr* payload = &texture;
                
                ImGui::SetDragDropPayload("TEXTURE", payload, 8);
                ImGui::Image(texture->nativeHandle, ImVec2(64,64));
                ImGui::Text("Texture: %s", entry.filename().string().c_str());

                ImGui::EndDragDropSource();
            }

            break;
        }

        case lustra::Asset::Type::Material:
        {
            auto material = std::dynamic_pointer_cast<lustra::MaterialAsset>(asset);
            
            DrawMaterialPreview(material, { 128.0f, 128.0f });

            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                lustra::MaterialAssetPtr* payload = &material;
                
                ImGui::SetDragDropPayload("MATERIAL", payload, 8);
                DrawMaterialPreview(material, { 64.0f, 64.0f });
                ImGui::Text("Material: %s", entry.filename().string().c_str());

                ImGui::EndDragDropSource();
            }

            break;
        }

        case lustra::Asset::Type::Model:
        {
            auto model = std::dynamic_pointer_cast<lustra::ModelAsset>(asset);
            
            ImGui::ImageButton("##Asset", modelIcon->nativeHandle, ImVec2(128.0f, 128.0f));

            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                lustra::ModelAssetPtr* payload = &model;
                
                ImGui::SetDragDropPayload("MODEL", payload, 8);
                ImGui::Image(modelIcon->nativeHandle, ImVec2(64,64));
                ImGui::Text("Model: %s", entry.filename().string().c_str());

                ImGui::EndDragDropSource();
            }

            break;
        }

        case lustra::Asset::Type::Script:
        {
            auto script = std::dynamic_pointer_cast<lustra::ScriptAsset>(asset);
            
            ImGui::ImageButton("##Asset", scriptIcon->nativeHandle, ImVec2(128.0f, 128.0f));

            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                lustra::ScriptAssetPtr* payload = &script;
                
                ImGui::SetDragDropPayload("SCRIPT", payload, 8);
                ImGui::Image(scriptIcon->nativeHandle, ImVec2(64,64));
                ImGui::Text("Script: %s", entry.filename().string().c_str());

                ImGui::EndDragDropSource();
            }

            break;
        }

        case lustra::Asset::Type::Scene:
        {
            auto scene = std::dynamic_pointer_cast<lustra::SceneAsset>(asset);

            ImGui::ImageButton("##Asset", sceneIcon->nativeHandle, ImVec2(128.0f, 128.0f));

            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                lustra::SceneAssetPtr* payload = &scene;
                
                ImGui::SetDragDropPayload("SCENE", payload, 8);
                ImGui::Image(scriptIcon->nativeHandle, ImVec2(64,64));
                ImGui::Text("Scene: %s", entry.filename().string().c_str());

                ImGui::EndDragDropSource();
            }

            break;
        }

        case lustra::Asset::Type::Sound:
        {
            auto sound = std::dynamic_pointer_cast<lustra::SoundAsset>(asset);
            ImGui::ImageButton("##Asset", soundIcon->nativeHandle, ImVec2(128.0f, 128.0f));

            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                lustra::SoundAssetPtr* payload = &sound;
                
                ImGui::SetDragDropPayload("SOUND", payload, 8);
                ImGui::Image(soundIcon->nativeHandle, ImVec2(64,64));
                ImGui::Text("Sound: %s", entry.filename().string().c_str());

                ImGui::EndDragDropSource();
            }

            break;
        }

        case lustra::Asset::Type::VertexShader:
        {
            auto shader = std::dynamic_pointer_cast<lustra::VertexShaderAsset>(asset);
            ImGui::ImageButton("##Asset", vertexShaderIcon->nativeHandle, ImVec2(128.0f, 128.0f));

            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                lustra::VertexShaderAssetPtr* payload = &shader;
                
                ImGui::SetDragDropPayload("VS", payload, 8);
                ImGui::Image(vertexShaderIcon->nativeHandle, ImVec2(64,64));
                ImGui::Text("Vertex shader: %s", entry.filename().string().c_str());

                ImGui::EndDragDropSource();
            }

            break;
        }

        case lustra::Asset::Type::FragmentShader:
        {
            auto shader = std::dynamic_pointer_cast<lustra::FragmentShaderAsset>(asset);
            ImGui::ImageButton("##Asset", fragmentShaderIcon->nativeHandle, ImVec2(128.0f, 128.0f));

            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                lustra::FragmentShaderAssetPtr* payload = &shader;
                
                ImGui::SetDragDropPayload("FS", payload, 8);
                ImGui::Image(fragmentShaderIcon->nativeHandle, ImVec2(64,64));
                ImGui::Text("Fragment shader: %s", entry.filename().string().c_str());

                ImGui::EndDragDropSource();
            }

            break;
        }

        default:
            break;
    }
}

void Editor::DrawUnloadedAsset(const std::filesystem::path& entry)
{
    auto assetType = lustra::GetAssetType(entry.extension().string());

    ImGui::ImageButton("##Asset", assetIcons[assetType]->nativeHandle, ImVec2(128.0f, 128.0f));

    if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
    {
        switch(assetType)
        {
            case lustra::Asset::Type::Texture:
                lustra::AssetManager::Get().Load<lustra::TextureAsset>(entry);
                break;

            case lustra::Asset::Type::Material:
                lustra::AssetManager::Get().Load<lustra::MaterialAsset>(entry);
                break;

            case lustra::Asset::Type::Model:
                lustra::AssetManager::Get().Load<lustra::ModelAsset>(entry);
                break;

            case lustra::Asset::Type::Script:
                lustra::AssetManager::Get().Load<lustra::ScriptAsset>(entry);
                break;

            case lustra::Asset::Type::Scene:
                lustra::Multithreading::Get().AddJob({ {},
                    [entry]() {
                        lustra::AssetManager::Get().Load<lustra::SceneAsset>(entry);
                    }
                });
                break;

            case lustra::Asset::Type::Sound:
                lustra::AssetManager::Get().Load<lustra::SoundAsset>(entry);
                break;

            case lustra::Asset::Type::VertexShader:
                lustra::AssetManager::Get().Load<lustra::VertexShaderAsset>(entry);
                break;

            case lustra::Asset::Type::FragmentShader:
                lustra::AssetManager::Get().Load<lustra::FragmentShaderAsset>(entry);
                break;

            default:
                break;
        }
    }
}

void Editor::DrawCreateAssetMenu(const std::filesystem::path& currentDirectory)
{
    static bool materialActive = false;
    static bool scriptActive = false;
    static bool sceneActive = false;

    if(ImGui::BeginPopupContextWindow("Create asset"))
    {
        materialActive = ImGui::MenuItem("Create material");
        scriptActive = ImGui::MenuItem("Create script");
        sceneActive = ImGui::MenuItem("Create scene");

        ImGui::EndPopup();
    }

    if(materialActive)
    {
        ImGui::OpenPopup("Create material");
        materialActive = DrawCreateMaterialMenu(currentDirectory);
    }
    else if(scriptActive)
    {
        ImGui::OpenPopup("Create script");
        scriptActive = DrawCreateScriptMenu(currentDirectory);
    }
    else if(sceneActive)
    {
        ImGui::OpenPopup("Create scene");
        sceneActive = DrawCreateSceneMenu(currentDirectory);
    }
}

bool Editor::DrawCreateMaterialMenu(const std::filesystem::path& currentDirectory)
{
    static std::string newMaterialName = "material.mat";
    static int uniqueId = 0;
    bool active = true;

    if(ImGui::BeginPopupModal("Create material", &active, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("Material name", &newMaterialName);

        if(ImGui::Button("OK", ImVec2(120, 0)))
        {
            auto newMaterial = std::make_shared<lustra::MaterialAsset>();
            newMaterial->path = currentDirectory / newMaterialName;
            newMaterial->loaded = true;

            lustra::AssetManager::Get().Write(newMaterial);
            
            newMaterialName = "material" + std::to_string(uniqueId++);

            ImGui::CloseCurrentPopup();

            active = false;
        }

        ImGui::SameLine();

        if(ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();

            active = false;
        }
        
        ImGui::EndPopup();
    }

    return active;
}

bool Editor::DrawCreateScriptMenu(const std::filesystem::path& currentDirectory)
{
    static std::string newScriptName = "script.as";
    static int uniqueId = 0;
    bool active = true;

    if(ImGui::BeginPopupModal("Create script", &active, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("Script name", &newScriptName);

        if(ImGui::Button("OK", ImVec2(120, 0)))
        {
            std::ofstream scriptFile(currentDirectory / newScriptName, std::ios::app);
            scriptFile <<
R"(Entity self;
Scene@ scene;

void Start()
{
}

void Update(float deltaTime)
{
}
)";
            scriptFile.close();
            system(std::string("code " + currentDirectory.string() + "/" + newScriptName).c_str());

            auto newScript = lustra::AssetManager::Get().Load<lustra::ScriptAsset>(currentDirectory / newScriptName);
            
            newScriptName = "script" + std::to_string(uniqueId++) + ".as";

            ImGui::CloseCurrentPopup();

            active = false;
        }

        ImGui::SameLine();

        if(ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();

            active = false;
        }
        
        ImGui::EndPopup();
    }

    return active;
}

bool Editor::DrawCreateSceneMenu(const std::filesystem::path& currentDirectory)
{
    static std::string newSceneName = "scene.scn";
    static int uniqueId = 0;
    bool active = true;

    if(ImGui::BeginPopupModal("Create scene", &active, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("Scene name", &newSceneName);

        if(ImGui::Button("OK", ImVec2(120, 0)))
        {
            lustra::SceneAssetPtr newScene = 
                std::make_shared<lustra::SceneAsset>(
                    std::make_shared<lustra::Scene>()
                );

            newScene->path = currentDirectory / newSceneName;

            lustra::AssetManager::Get().Write(newScene);
            
            newSceneName = "scene" + std::to_string(uniqueId++) + ".json";

            ImGui::CloseCurrentPopup();

            active = false;
        }

        ImGui::SameLine();

        if(ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();

            active = false;
        }
        
        ImGui::EndPopup();
    }

    return active;
}

void Editor::DrawMaterialEditor(lustra::MaterialAssetPtr material)
{
    static lustra::Timer saveTimer;

    ImGui::Begin("Material Editor");

    if((ImGui::Button("Save") || saveTimer.GetElapsedSeconds() > 3.0f) && ImGui::IsWindowFocused())
    {
        lustra::AssetManager::Get().Write(material);

        saveTimer.Reset();
    }

    ImGui::Separator();
    
    ImGui::Text("Albedo:");
    DrawMaterialProperty(material->albedo, 1);

    ImGui::Text("Normal:");
    DrawMaterialProperty(material->normal, 2);

    ImGui::Text("Metallic:");
    DrawMaterialProperty(material->metallic, 3, true);

    ImGui::Text("Roughness:");
    DrawMaterialProperty(material->roughness, 4, true);

    ImGui::Text("Ambient occlusion:");
    DrawMaterialProperty(material->ao, 5);

    ImGui::Text("Emission:");
    DrawMaterialProperty(material->emission, 6);

    ImGui::Text("Emission strength:");
    ImGui::DragFloat("##EmissionStrength", &material->emissionStrength, 0.01f, 0.0f, 100.0f);

    ImGui::Separator();
    ImGui::Text("UV Scale:");
    ImGui::DragFloat2("##UVScale", &material->uvScale.x, 0.01f, 0.0f, 100.0f);

    ImGui::Separator();
    ImGui::Text("UV Offset:");
    ImGui::DragFloat2("##UVOffset", &material->uvOffset.x, 0.01f, 0.0f, 100.0f);

    ImGui::End();
}

void Editor::DrawMaterialProperty(lustra::MaterialAsset::Property& property, int id, bool singleComponent)
{
    ImGui::PushID(id);

    if(property.type == lustra::MaterialAsset::Property::Type::Color)
    {
        if(singleComponent)
            ImGui::DragFloat("##Value", &property.value.x, 0.01f, 0.0f, 1.0f);
        else
            ImGui::ColorEdit4("##Color", &property.value.x);

        if(ImGui::Button("Set Texture"))
            property.type = lustra::MaterialAsset::Property::Type::Texture;
    }
    else
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        ImGui::Image(property.texture->nativeHandle, ImVec2(128.0f, 128.0f));

        ImGui::PopStyleVar();
        
        if(ImGui::BeginDragDropTarget())
        {
            auto payload = ImGui::AcceptDragDropPayload("TEXTURE");
            
            if(payload)
                property.texture = *(lustra::TextureAssetPtr*)payload->Data;

            ImGui::EndDragDropTarget();
        }

        if(ImGui::Button("Set Color"))
            property.type = lustra::MaterialAsset::Property::Type::Color;
    }

    ImGui::PopID();

    ImGui::Separator();
}

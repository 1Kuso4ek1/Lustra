#pragma once
#include <Components.hpp>

namespace dev
{

inline void DrawComponentUI(NameComponent& component, entt::entity entity)
{
    ImGui::InputText("Name", &component.name);
}

inline void DrawComponentUI(TransformComponent& component, entt::entity entity)
{
    ImGui::DragFloat3("Position", &component.position.x, 0.05f);
    ImGui::DragFloat3("Rotation", &component.rotation.x, 0.05f);
    ImGui::DragFloat3("Scale", &component.scale.x, 0.01f);

    if(ImGui::Button("Reset Scale"))
        component.scale = glm::vec3(1.0f);
}

inline void DrawComponentUI(MeshRendererComponent& component, entt::entity entity)
{
    float regionWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
    int cols = std::max(1, (int)(regionWidth / 128.0f));

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

    ImGui::Columns(cols, nullptr, false);

    for(size_t i = 0; i < component.materials.size(); i++)
    {
        ImGui::PushID(i);

        if(component.materials[i]->albedo.type == dev::MaterialAsset::Property::Type::Texture)
            ImGui::Image(component.materials[i]->albedo.texture->nativeHandle, ImVec2(128.0f, 128.0f));
        else
        {
            ImVec4 color = ImVec4(
                component.materials[i]->albedo.value.x,
                component.materials[i]->albedo.value.y,
                component.materials[i]->albedo.value.z,
                component.materials[i]->albedo.value.w
            );
            
            ImGui::PushStyleColor(ImGuiCol_Button, color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
            
            ImGui::Button("##Asset", ImVec2(128.0f, 128.0f));
            
            ImGui::PopStyleColor(3); 
        }

        if(ImGui::BeginDragDropTarget())
        {
            auto payload = ImGui::AcceptDragDropPayload("MATERIAL");

            if(payload)
                component.materials[i] = *(MaterialAssetPtr*)payload->Data;

            ImGui::EndDragDropTarget();
        }

        if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            dev::MaterialAssetPtr* payload = &component.materials[i];
            
            ImGui::SetDragDropPayload("MATERIAL", payload, 8);    
            ImGui::Image(component.materials[i]->albedo.texture->nativeHandle, ImVec2(64,64));
            ImGui::Text("Material");

            ImGui::EndDragDropSource();
        }

        ImGui::NextColumn();

        ImGui::PopID();
    }

    ImGui::NextColumn();

    ImGui::Columns(1);

    ImGui::PopStyleVar();

    ImGui::Separator();

    if(ImGui::Button("Add Material"))
        component.materials.push_back(component.materials.back());
}

inline void DrawComponentUI(CameraComponent& component, entt::entity entity)
{
    if(ImGui::DragFloat("FOV", &component.camera.fov, 0.05f, 1.0f, 179.0f))
        component.camera.SetPerspective();

    if(ImGui::DragFloat("Near", &component.camera.near, 0.05f, 0.01f, 10.0f))
        component.camera.SetPerspective();

    if(ImGui::DragFloat("Far", &component.camera.far, 0.05f, 0.01f, 5000.0f))
        component.camera.SetPerspective();
}

inline void DrawComponentUI(LightComponent& component, entt::entity entity)
{
    ImGui::ColorEdit3("Color", &component.color.x);
    ImGui::DragFloat("Intensity", &component.intensity, 0.05f, 0.0f, 100.0f);
    ImGui::DragFloat("Cutoff", &component.cutoff, 0.05f, 0.0f, 360.0f);
    ImGui::DragFloat("Outer Cutoff", &component.outerCutoff, 0.05f, 0.0f, 360.0f);
    ImGui::Checkbox("Shadow map", &component.shadowMap);
    
    uint32_t min = 128, max = 8192;

    ImGui::DragScalarN("Resolution", ImGuiDataType_U32, &component.resolution.width, 2, 1.0f, &min, &max);
    ImGui::DragFloat("Bias", &component.bias, 0.0001f, 0.0f, 1.0f);

    if(ImGui::Button("Setup Shadow Map"))
        component.SetupShadowMap(component.resolution);
}

inline void DrawComponentUI(TonemapComponent& component, entt::entity entity)
{
    static const std::vector<const char*> algorithms =
    {
        "ACES",
        "ACES Film",
        "Reinhard",
        "Uncharted 2",
        "Filmic",
        "Lottes"
    };

    ImGui::DragFloat("Exposure", &component.exposure, 0.05f, 0.0f, 100.0f);
    ImGui::Combo("Algorithm", &component.algorithm, algorithms.data(), algorithms.size());
}

inline void DrawComponentUI(BloomComponent& component, entt::entity entity)
{
    ImGui::DragFloat("Threshold", &component.threshold, 0.01f, 0.0f, 10.0f);
    ImGui::DragFloat("Strength", &component.strength, 0.01f, 0.0f, 10.0f);
}

inline void DrawComponentUI(ProceduralSkyComponent& component, entt::entity entity)
{
    ImGui::DragFloat("Time", &component.time, 0.1f, 0.0f, 1000.0f);
    ImGui::DragFloat("Cirrus", &component.cirrus, 0.001f, 0.0f, 1.0f);
    ImGui::DragFloat("Cumulus", &component.cumulus, 0.001f, 0.0f, 1.0f);
}

inline void DrawComponentUI(HDRISkyComponent& component, entt::entity entity)
{
    ImGui::Text("Environment Map:");
    ImGui::Image(component.environmentMap->nativeHandle, ImVec2(128.0f, 128.0f));

    if(ImGui::BeginDragDropTarget())
    {
        auto payload = ImGui::AcceptDragDropPayload("TEXTURE");
        
        if(payload)
        {
            component.environmentMap = *(TextureAssetPtr*)payload->Data;

            Multithreading::Get().AddJob({ {}, [&]() { component.Build(); } });
        }

        ImGui::EndDragDropTarget();
    }

    uint32_t min = 128, max = 8192;

    if(ImGui::DragScalar("Resolution", ImGuiDataType_U32, &component.resolution.width, 8.0f, &min, &max))
        component.resolution.height = component.resolution.width;

    if(ImGui::Button("Convert"))
        Multithreading::Get().AddJob({ {}, [&]() { component.Build(); } });
}

// Jesus Christ what is that
template<class T>
struct HasComponentUI
{
    template<class U> static char Test(void(*)(U&, entt::entity));
    template<class U> static int Test(...);

    static const bool value = sizeof(Test<T>(nullptr)) == sizeof(char);
};

// This is just magic
template <typename... Components>
void DrawEntityUI(entt::registry& registry, entt::entity entity)
{
    if(!registry.valid(entity))
        return;

    auto draw = [&]<typename Component>(Component* component)
    {
        ImGui::PushID((entt::id_type)entity);

        if constexpr (HasComponentUI<Component>::value)
            if(ImGui::CollapsingHeader(component->componentName.data()))
            {
                DrawComponentUI(*component, entity);
                
                ImGui::PushID(component->componentName.data());

                if(ImGui::Button("Remove"))
                    registry.remove<Component>(entity);

                ImGui::PopID();
            }

        ImGui::PopID();
    };

    ([&]<typename Component>()
    {
        Component* component = registry.try_get<Component>(entity);
        if(component)
            draw(component);
    }.template operator()<Components>(), ...); 
}

}

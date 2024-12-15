#pragma once
#include <TextureManager.hpp>
#include <Mesh.hpp>
#include <ImGuiManager.hpp>

#include <entt/entt.hpp>

namespace dev
{

struct NameComponent
{
    std::string name;
};

struct TransformComponent
{
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::quat rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
    glm::vec3 scale =    { 1.0f, 1.0f, 1.0f };

    glm::mat4 GetTransform() const;
};

struct MeshComponent
{
    std::vector<std::shared_ptr<Mesh>> meshes;
};

struct MaterialComponent
{
    std::vector<std::shared_ptr<TextureHandle>> albedo;
};

struct PipelineComponent
{
    LLGL::PipelineState* pipeline;
};

inline void DrawComponentUI(dev::NameComponent& component, entt::entity entity)
{
    if(ImGui::CollapsingHeader("NameComponent"))
        ImGui::InputText("Name", &component.name);
}

inline void DrawComponentUI(dev::TransformComponent& component, entt::entity entity)
{
    if(ImGui::CollapsingHeader("TransformComponent"))
    {
        ImGui::InputFloat3("Position", &component.position[0]);
        ImGui::InputFloat4("Rotation", &component.rotation[0]);
        ImGui::InputFloat3("Scale", &component.scale[0]);
    }
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
template <class... Components>
void DrawEntityUI(entt::registry& registry, entt::entity entity)
{
    auto Fold = [&]<class Component>(Component& c) 
    {
        if constexpr (HasComponentUI<Component>::value)
            DrawComponentUI(c, entity);
    };

    if(registry.valid(entity))
    {
        auto components = registry.get<Components...>(entity);

        std::apply([&](auto&... component)
        {
            (Fold(component), ...);
        }, components);
    }
}

}

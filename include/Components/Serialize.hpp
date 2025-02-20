#pragma once
#include <cereal/archives/json.hpp>

#include <Components.hpp>

namespace glm
{

template<class Archive>
void serialize(Archive& archive, vec2& vec)
{
    archive(vec.x, vec.y);
}

template<class Archive>
void serialize(Archive& archive, vec3& vec)
{
    archive(vec.x, vec.y, vec.z);
}

template<class Archive>
void serialize(Archive& archive, vec4& vec)
{
    archive(vec.x, vec.y, vec.z, vec.w);
}

}

namespace LLGL
{

template<class Archive>
void serialize(Archive& archive, Extent2D& extent)
{
    archive(extent.width, extent.height);
}

}

namespace entt
{

template<class Archive>
void serialize(Archive& archive, std::vector<entity>& entity)
{
    archive(entity);
}

}

namespace dev
{

template<class Archive>
void serialize(Archive& archive, NameComponent& component)
{
    archive(component.name);
}

template<class Archive>
void serialize(Archive& archive, TransformComponent& component)
{
    archive(component.position, component.rotation, component.scale);
}

template<class Archive>
void save(Archive& archive, const MeshComponent& component)
{
    archive(component.model->path.string());
}

template<class Archive>
void load(Archive& archive, MeshComponent& component)
{
    std::string path;

    archive(path);

    component.model = AssetManager::Get().Load<ModelAsset>(path);
}

template<class Archive>
void save(Archive& archive, const MeshRendererComponent& component)
{
    archive(component.materials.size());

    for(auto& material : component.materials)
        archive(material->path.string());
}

template<class Archive>
void load(Archive& archive, MeshRendererComponent& component)
{
    size_t size;

    archive(size);

    component.materials.resize(size);

    for(auto& material : component.materials)
    {
        std::string path;

        archive(path);

        material = AssetManager::Get().Load<MaterialAsset>(path);
    }
}

template<class Archive>
void save(Archive& archive, const PipelineComponent& component)
{
    archive(std::string("PipelineComponent"));
}

template<class Archive>
void load(Archive& archive, PipelineComponent& component)
{
    auto vertexShader = dev::Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, "../shaders/vertex.vert");
    auto fragmentShader = dev::Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, "../shaders/deferred.frag");

    component.pipeline = dev::Renderer::Get().CreatePipelineState(vertexShader, fragmentShader);
}

template<class Archive>
void serialize(Archive& archive, HierarchyComponent& component)
{
    archive(component.parent, component.children);
}

template<class Archive>
void serialize(Archive& archive, CameraComponent& component)
{
    archive(component.camera);
}

/* template<class Archive>
void serialize(Archive& archive, ScriptComponent& component)
{
    archive(component.script, component.moduleIndex, component.start, component.update);
} */

/* template<class Archive>
void serialize(Archive& archive, RigidBodyComponent& component)
{
    archive(component.body);
} */

}

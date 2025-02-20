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

namespace JPH
{

// Serialize vectors
template<class Archive>
void save(Archive& archive, const Vec3& vec)
{
    archive(vec.GetX(), vec.GetY(), vec.GetZ());
}

template<class Archive>
void load(Archive& archive, Vec3& vec)
{
    float x, y, z;
    archive(x, y, z);
    vec = { x, y, z };
}

template<class Archive>
void save(Archive& archive, const Quat& vec)
{
    archive(vec.GetX(), vec.GetY(), vec.GetZ(), vec.GetW());
}

template<class Archive>
void load(Archive& archive, Quat& vec)
{
    float x, y, z, w;
    archive(x, y, z, w);
    vec = { x, y, z, w };
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
    archive(component.vertexShader->path.string());
    archive(component.fragmentShader->path.string());
}

template<class Archive>
void load(Archive& archive, PipelineComponent& component)
{
    std::string vertexShaderPath, fragmentShaderPath;

    archive(vertexShaderPath, fragmentShaderPath);

    component.vertexShader = AssetManager::Get().Load<VertexShaderAsset>(vertexShaderPath);
    component.fragmentShader = AssetManager::Get().Load<FragmentShaderAsset>(fragmentShaderPath);

    component.pipeline = 
        dev::Renderer::Get().CreatePipelineState(
            component.vertexShader->shader,
            component.fragmentShader->shader
        );
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

template<class Archive>
void save(Archive& archive, const RigidBodyComponent& component)
{
    archive(component.body->GetPosition());
    archive(component.body->GetRotation());

    archive(component.body->GetMotionType());
    archive(component.body->IsSensor());

    archive(component.settings.type);

    switch(component.settings.type)
    {
    case RigidBodyComponent::ShapeSettings::Type::Empty:
        archive(component.settings.centerOfMass);
        break;

    case RigidBodyComponent::ShapeSettings::Type::Box:
        archive(component.settings.halfExtent);
        break;

    case RigidBodyComponent::ShapeSettings::Type::Sphere:
        archive(component.settings.radius);
        break;

    case RigidBodyComponent::ShapeSettings::Type::Capsule:
        archive(component.settings.radius, component.settings.halfHeight);
        break;

    case RigidBodyComponent::ShapeSettings::Type::Mesh:
        archive(component.settings.meshShape->path.string());
        break;
    }
}

template<class Archive>
void load(Archive& archive, RigidBodyComponent& component)
{
    JPH::Vec3 pos;
    JPH::Quat rot;

    archive(pos);
    archive(rot);

    JPH::EMotionType motionType;
    bool isSensor;

    archive(motionType);
    archive(isSensor);

    auto settings =
        JPH::BodyCreationSettings(
            new JPH::EmptyShapeSettings(),
            pos,
            rot,
            motionType,
            motionType == JPH::EMotionType::Dynamic || motionType == JPH::EMotionType::Kinematic ?
                Layers::moving : Layers::nonMoving
        );

    settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
    settings.mMassPropertiesOverride.mMass = 1.0f;

    component.body = PhysicsManager::Get().CreateBody(settings);

    component.body->SetIsSensor(isSensor);
    
    archive(component.settings.type);

    std::shared_ptr<JPH::ShapeSettings> shapeSettings;

    switch(component.settings.type)
    {
        case RigidBodyComponent::ShapeSettings::Type::Empty:
            archive(component.settings.centerOfMass);

            shapeSettings = std::make_shared<JPH::EmptyShapeSettings>();
            std::static_pointer_cast<JPH::EmptyShapeSettings>(shapeSettings)->
                mCenterOfMass = { 
                    component.settings.centerOfMass.x,
                    component.settings.centerOfMass.y,
                    component.settings.centerOfMass.z
                };

            break;

        case RigidBodyComponent::ShapeSettings::Type::Box:
            archive(component.settings.halfExtent);

            shapeSettings = std::make_shared<JPH::BoxShapeSettings>();
            std::static_pointer_cast<JPH::BoxShapeSettings>(shapeSettings)->
                mHalfExtent = { 
                    component.settings.halfExtent.x,
                    component.settings.halfExtent.y,
                    component.settings.halfExtent.z
                };

            break;

        case RigidBodyComponent::ShapeSettings::Type::Sphere:
            archive(component.settings.radius);

            shapeSettings = std::make_shared<JPH::SphereShapeSettings>();
            std::static_pointer_cast<JPH::SphereShapeSettings>(shapeSettings)
                ->mRadius = component.settings.radius;

            break;

        case RigidBodyComponent::ShapeSettings::Type::Capsule:
            archive(component.settings.radius, component.settings.halfHeight);

            shapeSettings = std::make_shared<JPH::CapsuleShapeSettings>();
            std::static_pointer_cast<JPH::CapsuleShapeSettings>(shapeSettings)->
                mRadius = component.settings.radius;
            std::static_pointer_cast<JPH::CapsuleShapeSettings>(shapeSettings)->
                mHalfHeightOfCylinder = component.settings.halfHeight;

            break;

        case RigidBodyComponent::ShapeSettings::Type::Mesh:
        {
            std::string path;
            archive(path);

            component.settings.meshShape = AssetManager::Get().Load<ModelAsset>(path);

            // Move this code somewhere so it doesn't repeat in 2 places
            JPH::TriangleList list;

            for(auto& mesh : component.settings.meshShape->meshes)
            {
                auto vertices = mesh->GetVertices();
                auto indices = mesh->GetIndices();

                list.reserve(list.size() + indices.size() / 3);

                for(size_t i = 0; i < indices.size(); i += 3)
                {
                    auto pos0 = vertices[indices[i]].position;
                    auto pos1 = vertices[indices[i + 1]].position;
                    auto pos2 = vertices[indices[i + 2]].position;

                    list.emplace_back(
                        JPH::Float3(pos0.x, pos0.y, pos0.z),
                        JPH::Float3(pos1.x, pos1.y, pos1.z),
                        JPH::Float3(pos2.x, pos2.y, pos2.z)
                    );
                }
            }

            shapeSettings = std::make_shared<JPH::MeshShapeSettings>(list);
            
            break;
        }
    }

    auto body = component.body;
    auto bodyId = body->GetID();

    PhysicsManager::Get().GetBodyInterface().SetShape(
        bodyId,
        shapeSettings->Create().Get(),
        false,
        JPH::EActivation::Activate
    );
}

}

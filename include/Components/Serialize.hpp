#pragma once
#include <cereal/archives/json.hpp>

#include <Components.hpp>

#include <ScriptManager.hpp>

namespace glm
{

template<class Archive>
void serialize(Archive& archive, vec2& vec)
{
    archive(cereal::make_nvp("x", vec.x), cereal::make_nvp("y", vec.y));
}

template<class Archive>
void serialize(Archive& archive, vec3& vec)
{
    archive(
        cereal::make_nvp("x", vec.x),
        cereal::make_nvp("y", vec.y),
        cereal::make_nvp("z", vec.z)
    );
}

template<class Archive>
void serialize(Archive& archive, vec4& vec)
{
    archive(
        cereal::make_nvp("x", vec.x),
        cereal::make_nvp("y", vec.y),
        cereal::make_nvp("z", vec.z),
        cereal::make_nvp("w", vec.w)
    );
}

}

namespace LLGL
{

template<class Archive>
void serialize(Archive& archive, Extent2D& extent)
{
    archive(cereal::make_nvp("width", extent.width), cereal::make_nvp("height", extent.height));
}

}

namespace JPH
{

template<class Archive>
void save(Archive& archive, const Vec3& vec)
{
    archive(
        cereal::make_nvp("x", vec.GetX()),
        cereal::make_nvp("y", vec.GetY()),
        cereal::make_nvp("z", vec.GetZ())
    );
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
    archive(
        cereal::make_nvp("x", vec.GetX()),
        cereal::make_nvp("y", vec.GetY()),
        cereal::make_nvp("z", vec.GetZ()),
        cereal::make_nvp("w", vec.GetW())
    );
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
void save(Archive& archive, const std::vector<entity>& entities)
{
    archive(cereal::make_nvp("size", entities.size()));

    for(auto& entity : entities)
        archive(cereal::make_nvp("entity", (uint32_t)entity));
}

template<class Archive>
void load(Archive& archive, std::vector<entity>& entities)
{
    size_t size;

    archive(size);

    entities.resize(size);

    for(auto& entity : entities)
        archive(entity);
}

}

namespace lustra
{

template<class Archive>
void serialize(Archive& archive, NameComponent& component)
{
    archive(cereal::make_nvp("name", component.name));
}

template<class Archive>
void serialize(Archive& archive, TransformComponent& component)
{
    archive(
        cereal::make_nvp("position", component.position),
        cereal::make_nvp("rotation", component.rotation),
        cereal::make_nvp("scale", component.scale),
        cereal::make_nvp("overridePhysics", component.overridePhysics)
    );
}

template<class Archive>
void save(Archive& archive, const MeshComponent& component)
{
    if(component.model)
        archive(cereal::make_nvp("modelPath", component.model->path.string()));
    else
        archive(cereal::make_nvp("modelPath", ""));
}

template<class Archive>
void load(Archive& archive, MeshComponent& component)
{
    std::string path;

    archive(path);

    if(!path.empty())
        component.model = AssetManager::Get().Load<ModelAsset>(path);
}

template<class Archive>
void save(Archive& archive, const MeshRendererComponent& component)
{
    archive(cereal::make_nvp("size", component.materials.size()));

    for(auto& material : component.materials)
        archive(cereal::make_nvp("materialPath", material->path.string()));
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
    archive(cereal::make_nvp("vertexShaderPath", component.vertexShader->path.string()));
    archive(cereal::make_nvp("fragmentShaderPath", component.fragmentShader->path.string()));
}

template<class Archive>
void load(Archive& archive, PipelineComponent& component)
{
    std::string vertexShaderPath, fragmentShaderPath;

    archive(vertexShaderPath, fragmentShaderPath);

    component.vertexShader = AssetManager::Get().Load<VertexShaderAsset>(vertexShaderPath);
    component.fragmentShader = AssetManager::Get().Load<FragmentShaderAsset>(fragmentShaderPath);

    component.pipeline = 
        Renderer::Get().CreatePipelineState(
            component.vertexShader->shader,
            component.fragmentShader->shader
        );
}

template<class Archive>
void serialize(Archive& archive, HierarchyComponent& component)
{
    archive(
        cereal::make_nvp("parent", component.parent),
        cereal::make_nvp("children", component.children)
    );
}

template<class Archive>
void serialize(Archive& archive, CameraComponent& component)
{
    archive(component.camera);
}

template<class Archive>
void save(Archive& archive, const ScriptComponent& component)
{
    archive(cereal::make_nvp("scriptPath", component.script ? component.script->path.string() : ""));
}

template<class Archive>
void load(Archive& archive, ScriptComponent& component)
{
    std::string path;

    archive(path);

    if(path.empty())
        return;

    component.script = AssetManager::Get().Load<ScriptAsset>(path);
    component.moduleIndex = component.script->modulesCount++;

    ScriptManager::Get().AddScript(component.script);

    ScriptManager::Get().Build();
}

template<class Archive>
void save(Archive& archive, const RigidBodyComponent& component)
{
    archive(cereal::make_nvp("rigidBodyPosition", component.body->GetPosition()));
    archive(cereal::make_nvp("rigidBodyRotation", component.body->GetRotation()));

    archive(cereal::make_nvp("rigidBodyMotionType", component.body->GetMotionType()));
    archive(cereal::make_nvp("rigidBodyIsSensor", component.body->IsSensor()));

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

template<class Archive>
void serialize(Archive& archive, Sound::Cone& cone)
{
    archive(
        cereal::make_nvp("outerAngle", cone.outer),
        cereal::make_nvp("innerAngle", cone.inner),
        cereal::make_nvp("outerGain", cone.outerGain)
    );
}

template<class Archive>
void save(Archive& archive, const SoundComponent& component)
{
    if(component.sound)
    {
        archive(cereal::make_nvp("soundPath", component.sound->path.string()));

        archive(cereal::make_nvp("cone", component.sound->sound.GetCone()));

        archive(cereal::make_nvp("directionalAttenuationFactor", component.sound->sound.GetDirectionalAttenuationFactor()));
        archive(cereal::make_nvp("dopplerFactor", component.sound->sound.GetDopplerFactor()));
        archive(cereal::make_nvp("rolloff", component.sound->sound.GetRolloff()));

        archive(cereal::make_nvp("maxDistance", component.sound->sound.GetMaxDistance()));
        archive(cereal::make_nvp("minDistance", component.sound->sound.GetMinDistance()));
        archive(cereal::make_nvp("maxGain", component.sound->sound.GetMaxGain()));
        archive(cereal::make_nvp("minGain", component.sound->sound.GetMinGain()));

        archive(cereal::make_nvp("pan", component.sound->sound.GetPan()));
        archive(cereal::make_nvp("pitch", component.sound->sound.GetPitch()));
        archive(cereal::make_nvp("volume", component.sound->sound.GetVolume()));

        archive(cereal::make_nvp("looping", component.sound->sound.IsLooping()));
        archive(cereal::make_nvp("spatializationEnabled", component.sound->sound.IsSpatializationEnabled()));
    }
    else
        archive(cereal::make_nvp("soundPath", ""));
}

template<class Archive>
void load(Archive& archive, SoundComponent& component)
{
    std::string path;

    archive(path);

    if(!path.empty())
    {
        component.sound = AssetManager::Get().Load<SoundAsset>(path);

        float data; Sound::Cone cone; bool enabled;

        archive(cone); component.sound->sound.SetCone(cone);

        archive(data); component.sound->sound.SetDirectionalAttenuationFactor(data);
        archive(data); component.sound->sound.SetDopplerFactor(data);
        archive(data); component.sound->sound.SetRolloff(data);

        archive(data); component.sound->sound.SetMaxDistance(data);
        archive(data); component.sound->sound.SetMinDistance(data);
        archive(data); component.sound->sound.SetMaxGain(data);
        archive(data); component.sound->sound.SetMinGain(data);

        archive(data); component.sound->sound.SetPan(data);
        archive(data); component.sound->sound.SetPitch(data);
        archive(data); component.sound->sound.SetVolume(data);
        
        archive(enabled); component.sound->sound.SetLooping(enabled);
        archive(enabled); component.sound->sound.SetSpatializationEnabled(enabled);
    }
}

}

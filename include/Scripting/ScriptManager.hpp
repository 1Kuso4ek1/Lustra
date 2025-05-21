#pragma once
#include <Singleton.hpp>
#include <ScriptAsset.hpp>

#include <angelscript.h>
#include <scriptbuilder.h>

#include <functional>

namespace lustra
{

class ScriptManager final : public Singleton<ScriptManager>
{
public:
    ~ScriptManager() override;

    void Build();

    void ExecuteFunction(
        const ScriptAssetPtr& script,
        std::string_view declaration,
        const std::function<void(asIScriptContext*)>& setArgs = nullptr,
        uint32_t moduleIndex = 0
    ) const;

    void AddScript(const ScriptAssetPtr& script);
    void RemoveScript(const ScriptAssetPtr& script);

    std::unordered_map<std::string, void*> GetGlobalVariables(const ScriptAssetPtr& script, uint32_t moduleIndex = 0) const;

public:
    void AddModule(std::string_view name);

    void AddFunction(std::string_view declaration, const asSFuncPtr& ptr, asECallConvTypes callType = asCALL_GENERIC) const;
    void AddProperty(std::string_view declaration, void* ptr) const;

    void AddValueType(
        std::string_view name,
        int size,
        uint32_t traits,
        const std::unordered_map<std::string_view, asSFuncPtr>& methods,
        const std::unordered_map<std::string_view, int>& properties
    ) const;

    void AddType(
        std::string_view name,
        int size,
        const std::unordered_map<std::string_view, asSFuncPtr>& methods,
        const std::unordered_map<std::string_view, int>& properties
    ) const;

    void AddTypeConstructor(std::string_view name, std::string_view declaration, const asSFuncPtr& ptr) const;
    void AddTypeDestructor(std::string_view name, std::string_view declaration, const asSFuncPtr& ptr) const;
    void AddTypeFactory(std::string_view name, std::string_view declaration, const asSFuncPtr& ptr) const;
    void AddEnum(std::string_view name, const std::vector<std::string_view>& values) const;
    void AddEnumValues(std::string_view name, const std::unordered_map<std::string_view, int>& values) const;

    void SetDefaultNamespace(std::string_view name) const;

    bool BuildModule(const ScriptAssetPtr& script, std::string_view name);

private:
    ScriptManager();

    friend class Singleton<ScriptManager>;

private:
    void DiscardModules() const;

private:
    void RegisterLog() const;

    void RegisterRandom() const;

    void RegisterVec2() const;
    void RegisterVec3() const;
    void RegisterVec4() const;
    void RegisterQuat() const;
    void RegisterMat4() const;
    void RegisterGLM() const;

    void RegisterBody() const;
    void RegisterRayCast() const;

    void RegisterExtent2D() const;

    void RegisterCamera() const;
    void RegisterSound() const;

    void RegisterKeyboard() const;
    void RegisterMouse() const;
    void RegisterInputManager() const;

    void RegisterScriptManager() const;

    void RegisterTextureAsset() const;
    void RegisterMaterialAsset() const;
    void RegisterModelAsset() const;
    void RegisterSceneAsset() const;
    void RegisterScriptAsset() const;
    void RegisterSoundAsset() const;
    void RegisterAssetManager() const;

    void RegisterWindowResizeEvent() const;
    void RegisterAssetLoadedEvent();
    void RegisterCollisionEvent() const;

    void RegisterTimer() const;

    void RegisterNameComponent() const;
    void RegisterTransformComponent() const;
    void RegisterMeshComponent() const;
    void RegisterMeshRendererComponent() const;
    void RegisterCameraComponent() const;
    void RegisterLightComponent() const;
    void RegisterScriptComponent() const;
    void RegisterBodyComponent() const;
    void RegisterSoundComponent() const;

    void RegisterProceduralSkyComponent() const;
    void RegisterHDRISkyComponent() const;

    void RegisterTonemapComponent() const;
    void RegisterBloomComponent() const;
    void RegisterGTAOComponent() const;
    void RegisterSSRComponent() const;

    void RegisterEntity() const;

    void RegisterScene() const;

private:
    std::vector<ScriptAssetPtr> scripts;

private:
    asIScriptEngine* engine;
    asIScriptContext* context;

    CScriptBuilder builder;
};

}

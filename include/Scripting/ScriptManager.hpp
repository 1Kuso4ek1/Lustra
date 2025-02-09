#pragma once
#include <Singleton.hpp>
#include <AngelscriptUtils.hpp>
#include <ScriptAsset.hpp>

namespace dev
{

class ScriptManager : public Singleton<ScriptManager>
{
public:
    ~ScriptManager();

    void Build();

    void ExecuteFunction(
        ScriptAssetPtr script,
        std::string_view declaration,
        std::function<void(asIScriptContext*)> setArgs = nullptr
    );

    void AddScript(ScriptAssetPtr script);
    void RemoveScript(ScriptAssetPtr script);

    std::unordered_map<std::string_view, void*> GetGlobalVariables(ScriptAssetPtr script);

public:
    void AddModule(std::string_view name);

    void AddFunction(std::string_view declaration, const asSFuncPtr& ptr, asECallConvTypes callType = asCALL_GENERIC);
    void AddProperty(std::string_view declaration, void* ptr);
    
    void AddValueType(
        std::string_view name,
        int size,
        int traits,
        std::unordered_map<std::string_view, asSFuncPtr> methods,
        std::unordered_map<std::string_view, int> properties
    );
    
    void AddType(
        std::string_view name,
        int size,
        std::unordered_map<std::string_view, asSFuncPtr> methods,
        std::unordered_map<std::string_view, int> properties
    );

    void AddTypeConstructor(std::string_view name, std::string_view declaration, const asSFuncPtr& ptr);
    void AddTypeDestructor(std::string_view name, std::string_view declaration, const asSFuncPtr& ptr);
    void AddTypeFactory(std::string_view name, std::string_view declaration, const asSFuncPtr& ptr);
    void AddEnum(std::string_view name, std::vector<std::string_view> values);
    void AddEnumValues(std::string_view name, std::unordered_map<std::string_view, int> values);
    
    void SetDefaultNamespace(std::string_view name);

private:
    ScriptManager();
    
    friend class Singleton<ScriptManager>;

private:
    void DiscardModules();

private:
    void RegisterLog();
    
    void RegisterVec2();
    void RegisterVec3();
    void RegisterVec4();
    void RegisterQuat();

    void RegisterExtent2D();

    void RegisterCamera();

    void RegisterKeyboard();
    void RegisterMouse();

    void RegisterNameComponent();
    void RegisterTransformComponent();
    void RegisterCameraComponent();
    void RegisterLightComponent();
    void RegisterEntity();

private:
    std::vector<ScriptAssetPtr> scripts;

private:
    asIScriptEngine* engine;
    asIScriptContext* context;

    CScriptBuilder builder;
};

}

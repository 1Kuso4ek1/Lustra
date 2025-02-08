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

    void ExecuteFunction(ScriptAssetPtr script, std::string_view declaration);

    void AddScript(ScriptAssetPtr script);
    void RemoveScript(ScriptAssetPtr script);

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
    
    void SetDefaultNamespace(std::string_view name);

private:
    ScriptManager();
    
    friend class Singleton<ScriptManager>;

private:
    void RegisterLog();

private:
    void DiscardModules();

private:
    std::vector<ScriptAssetPtr> scripts;

private:
    asIScriptEngine* engine;
    asIScriptContext* context;

    CScriptBuilder builder;
};

}

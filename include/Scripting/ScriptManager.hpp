#pragma once
#include <Singleton.hpp>
#include <AngelscriptUtils.hpp>

namespace dev
{

class ScriptManager : public Singleton<ScriptManager>
{
public:
    void AddFunction(std::string declaration, const asSFuncPtr& ptr, asECallConvTypes callType = asCALL_GENERIC);
    void AddProperty(std::string declaration, void* ptr);
    
    void AddValueType(
        std::string name,
        int size,
        int traits,
        std::unordered_map<std::string, asSFuncPtr> methods,
        std::unordered_map<std::string, int> properties
    );
    
    void AddType(
        std::string name,
        int size,
        std::unordered_map<std::string, asSFuncPtr> methods,
        std::unordered_map<std::string, int> properties
    );

    void AddTypeConstructor(std::string name, std::string declaration, const asSFuncPtr& ptr);
    void AddTypeDestructor(std::string name, std::string declaration, const asSFuncPtr& ptr);
    void AddTypeFactory(std::string name, std::string declaration, const asSFuncPtr& ptr);
    void AddEnum(std::string name, std::vector<std::string> values);
    
    void SetDefaultNamespace(std::string name);

private:
    ScriptManager();
    
    friend class Singleton<ScriptManager>;

private:
    asIScriptEngine* engine;
    asIScriptContext* context;

    CScriptBuilder builder;
};

}

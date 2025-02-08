#include <ScriptManager.hpp>
#include <Timer.hpp>

namespace dev
{

ScriptManager::ScriptManager()
{
    engine = asCreateScriptEngine();

    engine->SetMessageCallback(asFUNCTION(as::MessageCallback), 0, asCALL_CDECL);
    engine->SetEngineProperty(asEP_ALLOW_MULTILINE_STRINGS, true);

    context = engine->CreateContext();

    RegisterStdString(engine);
    RegisterScriptArray(engine, true);
    RegisterScriptMath(engine);
    RegisterScriptDictionary(engine);
    RegisterScriptDateTime(engine);
    RegisterScriptFile(engine);

    RegisterLog();
}

ScriptManager::~ScriptManager()
{
    context->Release();
    engine->Release();
}

void ScriptManager::Build()
{
    ScopedTimer timer("Script building");

    DiscardModules();

    bool buildSucceded = true;

    for(auto i : scripts)
    {
        builder.StartNewModule(engine, i->path.stem().string().c_str());
        builder.AddSectionFromFile(i->path.string().c_str());
        buildSucceded = (builder.BuildModule() >= 0);
    }

    if(buildSucceded)
        LLGL::Log::Printf(LLGL::Log::ColorFlags::BrightGreen, "Scripts built successfully\n");
    else
        LLGL::Log::Printf(LLGL::Log::ColorFlags::StdError, "Failed to build scripts\n");
}

void ScriptManager::ExecuteFunction(ScriptAssetPtr script, std::string_view declaration)
{
    auto module = engine->GetModule(script->path.stem().string().c_str());
    auto func = module->GetFunctionByDecl(declaration.data());

    if(func)
    {
        context->Prepare(func);
        context->Execute();
    }
}

void ScriptManager::AddScript(ScriptAssetPtr script)
{
    scripts.push_back(script);
}

void ScriptManager::RemoveScript(ScriptAssetPtr script)
{
    auto it = std::find(scripts.begin(), scripts.end(), script);

    if(it != scripts.end())
    {
        engine->DiscardModule((*it)->path.stem().string().c_str());

        scripts.erase(it);
    }
}

void ScriptManager::AddModule(std::string_view name)
{
    builder.StartNewModule(engine, name.data());
}

void ScriptManager::AddFunction(std::string_view declaration, const asSFuncPtr& ptr, asECallConvTypes callType)
{
    engine->RegisterGlobalFunction(declaration.data(), ptr, callType);
}

void ScriptManager::AddProperty(std::string_view declaration, void* ptr)
{
    engine->RegisterGlobalProperty(declaration.data(), ptr);
}

void ScriptManager::AddEnum(std::string_view name, std::vector<std::string_view> values)
{
    engine->RegisterEnum(name.data());
    for(int i = 0; i < values.size(); i++)
        engine->RegisterEnumValue(name.data(), values[i].data(), i);
}

void ScriptManager::AddValueType(
    std::string_view name,
    int size,
    int traits,
    std::unordered_map<std::string_view, asSFuncPtr> methods,
    std::unordered_map<std::string_view, int> properties
)
{
    engine->RegisterObjectType(name.data(), size, asOBJ_VALUE | traits);
    for(auto& i : methods)
        engine->RegisterObjectMethod(name.data(), i.first.data(), i.second, asCALL_GENERIC);
    for(auto& i : properties)
        engine->RegisterObjectProperty(name.data(), i.first.data(), i.second);
}

void ScriptManager::AddType(
    std::string_view name,
    int size,
    std::unordered_map<std::string_view, asSFuncPtr> methods,
    std::unordered_map<std::string_view, int> properties
)
{
    engine->RegisterObjectType(name.data(), size, asOBJ_REF | asOBJ_NOCOUNT);
    for(auto& i : methods)
        engine->RegisterObjectMethod(name.data(), i.first.data(), i.second, asCALL_GENERIC);
    for(auto& i : properties)
        engine->RegisterObjectProperty(name.data(), i.first.data(), i.second);
}

void ScriptManager::AddTypeConstructor(std::string_view name, std::string_view declaration, const asSFuncPtr& ptr)
{
    engine->RegisterObjectBehaviour(name.data(), asBEHAVE_CONSTRUCT, declaration.data(), ptr, asCALL_GENERIC);
}

void ScriptManager::AddTypeDestructor(std::string_view name, std::string_view declaration, const asSFuncPtr& ptr)
{
    engine->RegisterObjectBehaviour(name.data(), asBEHAVE_DESTRUCT, declaration.data(), ptr, asCALL_GENERIC);
}

void ScriptManager::AddTypeFactory(std::string_view name, std::string_view declaration, const asSFuncPtr& ptr)
{
    engine->RegisterObjectBehaviour(name.data(), asBEHAVE_FACTORY, declaration.data(), ptr, asCALL_GENERIC);
}

void ScriptManager::SetDefaultNamespace(std::string_view name)
{
    engine->SetDefaultNamespace(name.data());
}

void ScriptManager::RegisterLog()
{
    SetDefaultNamespace("Log");

    AddFunction("void Write(const string& in)", WRAP_FN_PR(as::Write, (const std::string&), void));

    SetDefaultNamespace("");
}

void ScriptManager::DiscardModules()
{
    for(auto i : scripts)
        engine->DiscardModule(i->path.stem().string().c_str());
}

}

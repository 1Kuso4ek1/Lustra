#include <ScriptManager.hpp>

namespace dev
{

ScriptManager::ScriptManager()
{
    engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);
    engine->SetEngineProperty(asEP_ALLOW_MULTILINE_STRINGS, true);

    context = engine->CreateContext();

    RegisterStdString(engine);
    RegisterScriptArray(engine, true);
    RegisterScriptMath(engine);
    RegisterScriptDictionary(engine);
    RegisterScriptDateTime(engine);
    RegisterScriptFile(engine);

    
}

}

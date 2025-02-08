#include <ScriptManager.hpp>
#include <Timer.hpp>
#include <Entity.hpp>
#include <Keyboard.hpp>
#include <Mouse.hpp>

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

    SetDefaultNamespace("Log");
    RegisterLog();

    SetDefaultNamespace("");
    RegisterExtent2D();

    SetDefaultNamespace("glm");
    RegisterVec2();
    RegisterVec3();
    RegisterVec4();
    RegisterQuat();

    SetDefaultNamespace("dev::Keyboard");
    RegisterKeyboard();

    SetDefaultNamespace("dev::Mouse");
    RegisterMouse();

    SetDefaultNamespace("dev");
    RegisterCamera();

    RegisterNameComponent();
    RegisterTransformComponent();
    RegisterCameraComponent();
    RegisterLightComponent();
    RegisterEntity();

    SetDefaultNamespace("");
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

void ScriptManager::ExecuteFunction(
    ScriptAssetPtr script,
    std::string_view declaration,
    std::function<void(asIScriptContext*)> setArgs
)
{
    auto module = engine->GetModule(script->path.stem().string().c_str());
    auto func = module->GetFunctionByDecl(declaration.data());

    if(func)
    {
        context->Prepare(func);

        if(setArgs)
            setArgs(context);

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

void ScriptManager::AddEnumValues(std::string_view name, std::unordered_map<std::string_view, int> values)
{
    engine->RegisterEnum(name.data());
    for(auto& i : values)
        engine->RegisterEnumValue(name.data(), i.first.data(), i.second);
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

void ScriptManager::DiscardModules()
{
    for(auto i : scripts)
        engine->DiscardModule(i->path.stem().string().c_str());
}

void ScriptManager::RegisterLog()
{
    AddFunction("void Write(const string& in)", WRAP_FN(as::Write));
}

void ScriptManager::RegisterVec2()
{
    AddValueType("vec2", sizeof(glm::vec2), asGetTypeTraits<glm::vec2>() | asOBJ_POD, {},
        {
            { "float x", asOFFSET(glm::vec2, x) },
            { "float y", asOFFSET(glm::vec2, y) }
        }
    );
}

void ScriptManager::RegisterVec3()
{
    AddValueType("vec3", sizeof(glm::vec3), asGetTypeTraits<glm::vec3>() | asOBJ_POD, {},
        {
            { "float x", asOFFSET(glm::vec3, x) },
            { "float y", asOFFSET(glm::vec3, y) },
            { "float z", asOFFSET(glm::vec3, z) }
        }
    );
}

void ScriptManager::RegisterVec4()
{
    AddValueType("vec4", sizeof(glm::vec4), asGetTypeTraits<glm::vec4>() | asOBJ_POD, {},
        {
            { "float x", asOFFSET(glm::vec4, x) },
            { "float y", asOFFSET(glm::vec4, y) },
            { "float z", asOFFSET(glm::vec4, z) },
            { "float w", asOFFSET(glm::vec4, w) }
        }
    );
}

void ScriptManager::RegisterQuat()
{
    AddValueType("quat", sizeof(glm::quat), asGetTypeTraits<glm::quat>() | asOBJ_POD, {},
        {
            { "float x", asOFFSET(glm::quat, x) },
            { "float y", asOFFSET(glm::quat, y) },
            { "float z", asOFFSET(glm::quat, z) },
            { "float w", asOFFSET(glm::quat, w) }
        }
    );
}

void ScriptManager::RegisterExtent2D()
{
    AddValueType("Extent2D", sizeof(LLGL::Extent2D), asGetTypeTraits<LLGL::Extent2D>() | asOBJ_POD, {},
        {
            { "uint32 width", asOFFSET(LLGL::Extent2D, width) },
            { "uint32 height", asOFFSET(LLGL::Extent2D, height) }
        }
    );
}

void ScriptManager::RegisterCamera()
{
    AddType("Camera", sizeof(Camera),
        {
            { "void SetPerspective()", WRAP_MFN(Camera, SetPerspective) },
            { "void SetOrthographic(float, float, float, float)", WRAP_MFN(Camera, SetOrthographic) },
            { "void SetViewport(const Extent2D& in)", WRAP_MFN(Camera, SetViewport) },

            { "void SetFov(float)", WRAP_MFN(Camera, SetFov) },
            { "void SetNear(float)", WRAP_MFN(Camera, SetNear) },
            { "void SetFar(float)", WRAP_MFN(Camera, SetFar) },

            { "void SetFirstPerson(bool)", WRAP_MFN(Camera, SetFirstPerson) },

            { "glm::vec2 GetViewport() const", WRAP_MFN(Camera, GetViewport) },
            
            { "float GetFov() const", WRAP_MFN(Camera, GetFov) },
            { "float GetNear() const", WRAP_MFN(Camera, GetNear) },
            { "float GetFar() const", WRAP_MFN(Camera, GetFar) },

            { "float GetAspect() const", WRAP_MFN(Camera, GetAspect) },

            { "bool IsFirstPerson() const", WRAP_MFN(Camera, IsFirstPerson) }
        }, {}
    );
}

void ScriptManager::RegisterKeyboard()
{
    AddFunction("bool IsKeyPressed(int)", WRAP_FN(Keyboard::IsKeyPressed));
    AddFunction("bool IsKeyReleased(int)", WRAP_FN(Keyboard::IsKeyReleased));
    AddFunction("bool IsKeyRepeated(int)", WRAP_FN(Keyboard::IsKeyRepeated));

    AddEnumValues("Key",
        { 
            { "Unknown", (int)(Keyboard::Key::Unknown) },
            { "Space", (int)(Keyboard::Key::Space) },
            { "Apostrophe", (int)(Keyboard::Key::Apostrophe) },
            { "Comma", (int)(Keyboard::Key::Comma) },
            { "Minus", (int)(Keyboard::Key::Minus) },
            { "Period", (int)(Keyboard::Key::Period) },
            { "Slash", (int)(Keyboard::Key::Slash) },
            { "Num0", (int)(Keyboard::Key::Num0) },
            { "Num1", (int)(Keyboard::Key::Num1) },
            { "Num2", (int)(Keyboard::Key::Num2) },
            { "Num3", (int)(Keyboard::Key::Num3) },
            { "Num4", (int)(Keyboard::Key::Num4) },
            { "Num5", (int)(Keyboard::Key::Num5) },
            { "Num6", (int)(Keyboard::Key::Num6) },
            { "Num7", (int)(Keyboard::Key::Num7) },
            { "Num8", (int)(Keyboard::Key::Num8) },
            { "Num9", (int)(Keyboard::Key::Num9) },
            { "Semicolon", (int)(Keyboard::Key::Semicolon) },
            { "Equal", (int)(Keyboard::Key::Equal) },
            { "A", (int)(Keyboard::Key::A) },
            { "B", (int)(Keyboard::Key::B) },
            { "C", (int)(Keyboard::Key::C) },
            { "D", (int)(Keyboard::Key::D) },
            { "E", (int)(Keyboard::Key::E) },
            { "F", (int)(Keyboard::Key::F) },
            { "G", (int)(Keyboard::Key::G) },
            { "H", (int)(Keyboard::Key::H) },
            { "I", (int)(Keyboard::Key::I) },
            { "J", (int)(Keyboard::Key::J) },
            { "K", (int)(Keyboard::Key::K) },
            { "L", (int)(Keyboard::Key::L) },
            { "M", (int)(Keyboard::Key::M) },
            { "N", (int)(Keyboard::Key::N) },
            { "O", (int)(Keyboard::Key::O) },
            { "P", (int)(Keyboard::Key::P) },
            { "Q", (int)(Keyboard::Key::Q) },
            { "R", (int)(Keyboard::Key::R) },
            { "S", (int)(Keyboard::Key::S) },
            { "T", (int)(Keyboard::Key::T) },
            { "U", (int)(Keyboard::Key::U) },
            { "V", (int)(Keyboard::Key::V) },
            { "W", (int)(Keyboard::Key::W) },
            { "X", (int)(Keyboard::Key::X) },
            { "Y", (int)(Keyboard::Key::Y) },
            { "Z", (int)(Keyboard::Key::Z) },
            { "LeftBracket", (int)(Keyboard::Key::LeftBracket) },
            { "Backslash", (int)(Keyboard::Key::Backslash) },
            { "RightBracket", (int)(Keyboard::Key::RightBracket) },
            { "GraveAccent", (int)(Keyboard::Key::GraveAccent) },
            { "World1", (int)(Keyboard::Key::World1) },
            { "World2", (int)(Keyboard::Key::World2) },
            { "Escape", (int)(Keyboard::Key::Escape) },
            { "Enter", (int)(Keyboard::Key::Enter) },
            { "Tab", (int)(Keyboard::Key::Tab) },
            { "Backspace", (int)(Keyboard::Key::Backspace) },
            { "Insert", (int)(Keyboard::Key::Insert) },
            { "Delete", (int)(Keyboard::Key::Delete) },
            { "Right", (int)(Keyboard::Key::Right) },
            { "Left", (int)(Keyboard::Key::Left) },
            { "Down", (int)(Keyboard::Key::Down) },
            { "Up", (int)(Keyboard::Key::Up) },
            { "PageUp", (int)(Keyboard::Key::PageUp) },
            { "PageDown", (int)(Keyboard::Key::PageDown) },
            { "Home", (int)(Keyboard::Key::Home) },
            { "End", (int)(Keyboard::Key::End) },
            { "CapsLock", (int)(Keyboard::Key::CapsLock) },
            { "ScrollLock", (int)(Keyboard::Key::ScrollLock) },
            { "NumLock", (int)(Keyboard::Key::NumLock) },
            { "PrintScreen", (int)(Keyboard::Key::PrintScreen) },
            { "Pause", (int)(Keyboard::Key::Pause) },
            { "F1", (int)(Keyboard::Key::F1) },
            { "F2", (int)(Keyboard::Key::F2) },
            { "F3", (int)(Keyboard::Key::F3) },
            { "F4", (int)(Keyboard::Key::F4) },
            { "F5", (int)(Keyboard::Key::F5) },
            { "F6", (int)(Keyboard::Key::F6) },
            { "F7", (int)(Keyboard::Key::F7) },
            { "F8", (int)(Keyboard::Key::F8) },
            { "F9", (int)(Keyboard::Key::F9) },
            { "F10", (int)(Keyboard::Key::F10) },
            { "F11", (int)(Keyboard::Key::F11) },
            { "F12", (int)(Keyboard::Key::F12) },
            { "F13", (int)(Keyboard::Key::F13) },
            { "F14", (int)(Keyboard::Key::F14) },
            { "F15", (int)(Keyboard::Key::F15) },
            { "F16", (int)(Keyboard::Key::F16) },
            { "F17", (int)(Keyboard::Key::F17) },
            { "F18", (int)(Keyboard::Key::F18) },
            { "F19", (int)(Keyboard::Key::F19) },
            { "F20", (int)(Keyboard::Key::F20) },
            { "F21", (int)(Keyboard::Key::F21) },
            { "F22", (int)(Keyboard::Key::F22) },
            { "F23", (int)(Keyboard::Key::F23) },
            { "F24", (int)(Keyboard::Key::F24) },
            { "F25", (int)(Keyboard::Key::F25) },
            { "Kp0", (int)(Keyboard::Key::Kp0) },
            { "Kp1", (int)(Keyboard::Key::Kp1) },
            { "Kp2", (int)(Keyboard::Key::Kp2) },
            { "Kp3", (int)(Keyboard::Key::Kp3) },
            { "Kp4", (int)(Keyboard::Key::Kp4) },
            { "Kp5", (int)(Keyboard::Key::Kp5) },
            { "Kp6", (int)(Keyboard::Key::Kp6) },
            { "Kp7", (int)(Keyboard::Key::Kp7) },
            { "Kp8", (int)(Keyboard::Key::Kp8) },
            { "Kp9", (int)(Keyboard::Key::Kp9) },
            { "KpDecimal", (int)(Keyboard::Key::KpDecimal) },
            { "KpDivide", (int)(Keyboard::Key::KpDivide) },
            { "KpMultiply", (int)(Keyboard::Key::KpMultiply) },
            { "KpSubtract", (int)(Keyboard::Key::KpSubtract) },
            { "KpAdd", (int)(Keyboard::Key::KpAdd) },
            { "KpEnter", (int)(Keyboard::Key::KpEnter) },
            { "KpEqual", (int)(Keyboard::Key::KpEqual) },
            { "LeftShift", (int)(Keyboard::Key::LeftShift) },
            { "LeftControl", (int)(Keyboard::Key::LeftControl) },
            { "LeftAlt", (int)(Keyboard::Key::LeftAlt) },
            { "LeftSuper", (int)(Keyboard::Key::LeftSuper) },
            { "RightShift", (int)(Keyboard::Key::RightShift) },
            { "RightControl", (int)(Keyboard::Key::RightControl) },
            { "RightAlt", (int)(Keyboard::Key::RightAlt) },
            { "RightSuper", (int)(Keyboard::Key::RightSuper) },
            { "Menu", (int)(Keyboard::Key::Menu) },
            { "Last", (int)(Keyboard::Key::Last) }
        }
    );
}

void ScriptManager::RegisterMouse()
{
    AddFunction("glm::vec2 GetPosition()", WRAP_FN(Mouse::GetPosition));
    AddFunction("void SetPosition(const glm::vec2& in)", WRAP_FN(Mouse::SetPosition));
    AddFunction("bool IsButtonPressed(int)", WRAP_FN(Mouse::IsButtonPressed));
    AddFunction("bool IsButtonReleased(int)", WRAP_FN(Mouse::IsButtonReleased));

    AddEnumValues("Button",
        {
            { "Left", (int)(Mouse::Button::Left) },
            { "Right", (int)(Mouse::Button::Right) },
            { "Middle", (int)(Mouse::Button::Middle) },
            { "MButton4", (int)(Mouse::Button::MButton4) },
            { "MButton5", (int)(Mouse::Button::MButton5) },
            { "MButton6", (int)(Mouse::Button::MButton6) },
            { "MButton7", (int)(Mouse::Button::MButton7) },
            { "MButton8", (int)(Mouse::Button::MButton8) },
            { "Last", (int)(Mouse::Button::Last) }
        }
    );
}

void ScriptManager::RegisterNameComponent()
{
    AddType("NameComponent", sizeof(NameComponent), {},
        {
            { "string name", asOFFSET(NameComponent, name) }
        }
    );
}

void ScriptManager::RegisterTransformComponent()
{
    AddType("TransformComponent", sizeof(TransformComponent), {},
        {
            { "glm::vec3 position", asOFFSET(TransformComponent, position) },
            { "glm::vec3 rotation", asOFFSET(TransformComponent, rotation) },
            { "glm::vec3 scale", asOFFSET(TransformComponent, scale) }
        }
    );
}

void ScriptManager::RegisterCameraComponent()
{
    AddType("CameraComponent", sizeof(CameraComponent),
        {
            { "Camera@ get_camera() property", WRAP_OBJ_LAST(as::GetCamera) }
        }, {}
    );
}

void ScriptManager::RegisterLightComponent()
{
    AddType("LightComponent", sizeof(LightComponent),
        {
            { "void SetupShadowMap(const Extent2D& in)", WRAP_MFN_PR(LightComponent, SetupShadowMap, (const LLGL::Extent2D&), void) }
        },
        {
            { "glm::vec3 color", asOFFSET(LightComponent, color) },
            { "float intensity", asOFFSET(LightComponent, intensity) },
            { "float cutoff", asOFFSET(LightComponent, cutoff) },
            { "float outerCutoff", asOFFSET(LightComponent, outerCutoff) },
            { "bool shadowMap", asOFFSET(LightComponent, shadowMap) }
        }
    );
}

void ScriptManager::RegisterEntity()
{
    // It will be updated as soon as the Angelscript developer will write some docs on function templates
    AddType("Entity", sizeof(Entity),
        {
            { "NameComponent@ GetNameComponent()", WRAP_MFN(Entity, GetComponent<NameComponent>) },
            { "TransformComponent@ GetTransformComponent()", WRAP_MFN(Entity, GetComponent<TransformComponent>) },
            { "LightComponent@ GetLightComponent()", WRAP_MFN(Entity, GetComponent<LightComponent>) },
            { "CameraComponent@ GetCameraComponent()", WRAP_MFN(Entity, GetComponent<CameraComponent>) }
        }, {}
    );
}

}

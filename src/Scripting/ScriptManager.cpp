#include <AngelscriptUtils.hpp>
#include <Entity.hpp>
#include <Keyboard.hpp>
#include <Mouse.hpp>
#include <SceneAsset.hpp>
#include <Timer.hpp>

namespace lustra
{

ScriptManager::ScriptManager()
{
    engine = asCreateScriptEngine();

    engine->SetMessageCallback(asFUNCTION(as::MessageCallback), nullptr, asCALL_CDECL);
    engine->SetEngineProperty(asEP_ALLOW_MULTILINE_STRINGS, true);

    context = engine->CreateContext();

    RegisterStdString(engine);
    RegisterScriptArray(engine, true);
    RegisterScriptMath(engine);
    RegisterScriptDictionary(engine);
    RegisterScriptDateTime(engine);
    RegisterScriptFile(engine);

    RegisterExtent2D();

    SetDefaultNamespace("Log");
    RegisterLog();

    SetDefaultNamespace("Random");
    RegisterRandom();

    SetDefaultNamespace("glm");
    RegisterGLM();

    SetDefaultNamespace("JPH");
    RegisterBody();
    RegisterRayCast();

    SetDefaultNamespace("Keyboard");
    RegisterKeyboard();

    SetDefaultNamespace("Mouse");
    RegisterMouse();

    SetDefaultNamespace("InputManager");
    RegisterInputManager();

    SetDefaultNamespace("");
    RegisterSound();

    RegisterTextureAsset();
    RegisterMaterialAsset();
    RegisterModelAsset();
    RegisterScriptAsset();
    RegisterSoundAsset();

    SetDefaultNamespace("AssetManager");
    RegisterAssetManager();

    SetDefaultNamespace("ScriptManager");
    RegisterScriptManager();

    SetDefaultNamespace("");
    RegisterCamera();
    RegisterTimer();

    RegisterWindowResizeEvent();
    RegisterAssetLoadedEvent();
    RegisterCollisionEvent();

    RegisterNameComponent();
    RegisterTransformComponent();
    RegisterMeshComponent();
    RegisterMeshRendererComponent();
    RegisterCameraComponent();
    RegisterLightComponent();
    RegisterScriptComponent();
    RegisterBodyComponent();
    RegisterSoundComponent();

    RegisterProceduralSkyComponent();
    RegisterHDRISkyComponent();

    RegisterTonemapComponent();
    RegisterBloomComponent();
    RegisterGTAOComponent();
    RegisterSSRComponent();

    RegisterEntity();

    RegisterScene();

    RegisterSceneAsset();

    AddFunction("SceneAssetPtr LoadScene(const string& in, bool = false)", WRAP_FN(as::Load<SceneAsset>));
}

ScriptManager::~ScriptManager()
{
    DiscardModules();

    context->Release();
    engine->Release();
}

void ScriptManager::Build()
{
    ScopedTimer timer("Script building");

    DiscardModules();

    bool buildSucceded = true;

    for(const auto& i : scripts)
    {
        for(int j = 0; j < i->modulesCount; j++)
        {
            buildSucceded = BuildModule(i, (i->path.stem().string() + std::to_string(j)));
        }
    }

    if(buildSucceded)
        LLGL::Log::Printf(LLGL::Log::ColorFlags::BrightGreen, "Scripts built successfully\n");
    else
        LLGL::Log::Printf(LLGL::Log::ColorFlags::StdError, "Failed to build scripts\n");
}

void ScriptManager::ExecuteFunction(
    const ScriptAssetPtr& script,
    const std::string_view declaration,
    const std::function<void(asIScriptContext*)>& setArgs,
    const uint32_t moduleIndex
) const
{
    const auto module = engine->GetModule((script->path.stem().string() + std::to_string(moduleIndex)).c_str());

    if(const auto func = module->GetFunctionByDecl(declaration.data()))
    {
        if(context->GetState() == asEContextState::asEXECUTION_ACTIVE)
            LLGL::Log::Printf(
                LLGL::Log::ColorFlags::StdError,
                "Context is already in use\n"
            );

        context->Prepare(func);

        if(setArgs)
            setArgs(context);

        context->Execute();
        context->Unprepare();
    }
}

void ScriptManager::AddScript(const ScriptAssetPtr& script)
{
    if(std::ranges::find(scripts, script) == scripts.end())
        scripts.push_back(script);
}

void ScriptManager::RemoveScript(const ScriptAssetPtr& script)
{
    const auto it = std::ranges::find(scripts, script);

    if(it != scripts.end())
    {
        for(int i = 0; i < (*it)->modulesCount; i++)
            engine->DiscardModule(((*it)->path.stem().string() + std::to_string(i)).c_str());

        scripts.erase(it);
    }
}

std::unordered_map<std::string, void*> ScriptManager::GetGlobalVariables(const ScriptAssetPtr& script, const uint32_t moduleIndex) const
{
    std::unordered_map<std::string, void*> variables;

    const auto module = engine->GetModule((script->path.stem().string() + std::to_string(moduleIndex)).c_str());
    const auto count = module->GetGlobalVarCount();

    for(int i = 0; i < count; i++)
    {
        const auto var = module->GetGlobalVarDeclaration(i);
        variables[var] = module->GetAddressOfGlobalVar(i);
    }

    return variables;
}

void ScriptManager::AddModule(const std::string_view name)
{
    builder.StartNewModule(engine, name.data());
}

void ScriptManager::AddFunction(const std::string_view declaration, const asSFuncPtr& ptr, const asECallConvTypes callType) const
{
    engine->RegisterGlobalFunction(declaration.data(), ptr, callType);
}

void ScriptManager::AddProperty(const std::string_view declaration, void* ptr) const
{
    engine->RegisterGlobalProperty(declaration.data(), ptr);
}

void ScriptManager::AddEnum(const std::string_view name, const std::vector<std::string_view>& values) const
{
    engine->RegisterEnum(name.data());
    for(int i = 0; i < values.size(); i++)
        engine->RegisterEnumValue(name.data(), values[i].data(), i);
}

void ScriptManager::AddEnumValues(const std::string_view name, const std::unordered_map<std::string_view, int>& values) const
{
    engine->RegisterEnum(name.data());
    for(const auto& [entry, value] : values)
        engine->RegisterEnumValue(name.data(), entry.data(), value);
}

void ScriptManager::AddValueType(
    const std::string_view name,
    const int size,
    const uint32_t traits,
    const std::unordered_map<std::string_view, asSFuncPtr>& methods,
    const std::unordered_map<std::string_view, int>& properties
) const
{
    engine->RegisterObjectType(name.data(), size, asOBJ_VALUE | traits);
    for(const auto& [entry, method] : methods)
        engine->RegisterObjectMethod(name.data(), entry.data(), method, asCALL_GENERIC);
    for(const auto& [entry, property] : properties)
        engine->RegisterObjectProperty(name.data(), entry.data(), property);
}

void ScriptManager::AddType(
    const std::string_view name,
    const int size,
    const std::unordered_map<std::string_view, asSFuncPtr>& methods,
    const std::unordered_map<std::string_view, int>& properties
) const
{
    engine->RegisterObjectType(name.data(), size, asOBJ_REF | asOBJ_NOCOUNT);
    for(const auto& [entry, method] : methods)
        engine->RegisterObjectMethod(name.data(), entry.data(), method, asCALL_GENERIC);
    for(const auto& [entry, property] : properties)
        engine->RegisterObjectProperty(name.data(), entry.data(), property);
}

void ScriptManager::AddTypeConstructor(const std::string_view name, const std::string_view declaration, const asSFuncPtr& ptr) const
{
    engine->RegisterObjectBehaviour(name.data(), asBEHAVE_CONSTRUCT, declaration.data(), ptr, asCALL_GENERIC);
}

void ScriptManager::AddTypeDestructor(const std::string_view name, const std::string_view declaration, const asSFuncPtr& ptr) const
{
    engine->RegisterObjectBehaviour(name.data(), asBEHAVE_DESTRUCT, declaration.data(), ptr, asCALL_GENERIC);
}

void ScriptManager::AddTypeFactory(const std::string_view name, const std::string_view declaration, const asSFuncPtr& ptr) const
{
    engine->RegisterObjectBehaviour(name.data(), asBEHAVE_FACTORY, declaration.data(), ptr, asCALL_GENERIC);
}

void ScriptManager::SetDefaultNamespace(const std::string_view name) const
{
    engine->SetDefaultNamespace(name.data());
}

bool ScriptManager::BuildModule(const ScriptAssetPtr& script, const std::string_view name)
{
    AddModule(name);
    builder.AddSectionFromFile(script->path.string().c_str());

    return (builder.BuildModule() >= 0);
}

void ScriptManager::DiscardModules() const
{
    for(auto& i : scripts)
        for(int j = 0; j < i->modulesCount; j++)
            engine->DiscardModule((i->path.stem().string() + std::to_string(j)).c_str());
}

void ScriptManager::RegisterLog() const
{
    AddFunction("void Write(const string& in)", WRAP_FN(as::Write));
}

void ScriptManager::RegisterRandom() const
{
    AddFunction("void SetSeed(uint32)", WRAP_FN(as::RandomSetSeed));
    AddFunction("float Value()", WRAP_FN(as::RandomValue));
    AddFunction("float Range(float, float)", WRAP_FN_PR(as::RandomRange, (float, float), float));
    AddFunction("int Range(int, int)", WRAP_FN_PR(as::RandomRange, (int, int), int));
}

void ScriptManager::RegisterVec2() const
{
    AddValueType("vec2", sizeof(glm::vec2), asGetTypeTraits<glm::vec2>() | asOBJ_POD,
        {
            { "vec2 opAdd(const vec2& in)", WRAP_OBJ_FIRST_PR(glm::operator+, (const glm::vec2&, const glm::vec2&), glm::vec2) },
            { "vec2 opSub(const vec2& in)", WRAP_OBJ_FIRST_PR(glm::operator-, (const glm::vec2&, const glm::vec2&), glm::vec2) },
            { "vec2 opMul(const vec2& in)", WRAP_OBJ_FIRST_PR(glm::operator*, (const glm::vec2&, const glm::vec2&), glm::vec2) },
            { "vec2 opDiv(const vec2& in)", WRAP_OBJ_FIRST_PR(glm::operator/, (const glm::vec2&, const glm::vec2&), glm::vec2) },
            { "vec2 opMul(float)", WRAP_OBJ_FIRST_PR(glm::operator*, (const glm::vec2&, float), glm::vec2) },
            { "vec2 opDiv(float)", WRAP_OBJ_FIRST_PR(glm::operator/, (const glm::vec2&, float), glm::vec2) },
            { "vec2 opAddAssign(const vec2& in)", WRAP_OBJ_LAST(as::Vec2AddAssign) },
            { "vec2 opSubAssign(const vec2& in)", WRAP_OBJ_LAST(as::Vec2SubAssign) },
            { "vec2 opMulAssign(const vec2& in)", WRAP_OBJ_LAST(as::Vec2MulAssign) },
            { "vec2 opDivAssign(const vec2& in)", WRAP_OBJ_LAST(as::Vec2DivAssign) },
            { "vec2 opMulAssign(const vec2& in)", WRAP_OBJ_LAST(as::Vec2MulAssignScalar) },
            { "vec2 opDivAssign(const vec2& in)", WRAP_OBJ_LAST(as::Vec2DivAssignScalar) }
        },
        {
            { "float x", asOFFSET(glm::vec2, x) },
            { "float y", asOFFSET(glm::vec2, y) }
        }
    );

    AddTypeConstructor("vec2", "void f(float)", WRAP_OBJ_LAST(as::MakeVec2Scalar));
    AddTypeConstructor("vec2", "void f(float, float)", WRAP_OBJ_LAST(as::MakeVec2));

    AddFunction("float length(const vec2& in)", WRAP_FN_PR(glm::length, (const glm::vec2&), float));
    AddFunction("vec2 normalize(const vec2& in)", WRAP_FN_PR(glm::normalize, (const glm::vec2&), glm::vec2));
    AddFunction("float dot(const vec2& in, const vec2& in)", WRAP_FN_PR(glm::dot, (const glm::vec2&, const glm::vec2&), float));
}

void ScriptManager::RegisterVec3() const
{
    AddValueType("vec3", sizeof(glm::vec3), asGetTypeTraits<glm::vec3>() | asOBJ_POD,
        {
            { "vec3 opAdd(const vec3& in)", WRAP_OBJ_FIRST_PR(glm::operator+, (const glm::vec3&, const glm::vec3&), glm::vec3) },
            { "vec3 opSub(const vec3& in)", WRAP_OBJ_FIRST_PR(glm::operator-, (const glm::vec3&, const glm::vec3&), glm::vec3) },
            { "vec3 opMul(const vec3& in)", WRAP_OBJ_FIRST_PR(glm::operator*, (const glm::vec3&, const glm::vec3&), glm::vec3) },
            { "vec3 opDiv(const vec3& in)", WRAP_OBJ_FIRST_PR(glm::operator/, (const glm::vec3&, const glm::vec3&), glm::vec3) },
            { "vec3 opMul(float)", WRAP_OBJ_FIRST_PR(glm::operator*, (const glm::vec3&, float), glm::vec3) },
            { "vec3 opDiv(float)", WRAP_OBJ_FIRST_PR(glm::operator/, (const glm::vec3&, float), glm::vec3) },
            { "vec3 opAddAssign(const vec3& in)", WRAP_OBJ_LAST(as::Vec3AddAssign) },
            { "vec3 opSubAssign(const vec3& in)", WRAP_OBJ_LAST(as::Vec3SubAssign) },
            { "vec3 opMulAssign(const vec3& in)", WRAP_OBJ_LAST(as::Vec3MulAssign) },
            { "vec3 opDivAssign(const vec3& in)", WRAP_OBJ_LAST(as::Vec3DivAssign) },
            { "vec3 opMulAssign(const vec3& in)", WRAP_OBJ_LAST(as::Vec3MulAssignScalar) },
            { "vec3 opDivAssign(const vec3& in)", WRAP_OBJ_LAST(as::Vec3DivAssignScalar) }
        },
        {
            { "float x", asOFFSET(glm::vec3, x) },
            { "float y", asOFFSET(glm::vec3, y) },
            { "float z", asOFFSET(glm::vec3, z) }
        }
    );

    AddTypeConstructor("vec3", "void f(float)", WRAP_OBJ_LAST(as::MakeVec3Scalar));
    AddTypeConstructor("vec3", "void f(float, float, float)", WRAP_OBJ_LAST(as::MakeVec3));

    AddFunction("float length(const vec3& in)", WRAP_FN_PR(glm::length, (const glm::vec3&), float));
    AddFunction("vec3 normalize(const vec3& in)", WRAP_FN_PR(glm::normalize, (const glm::vec3&), glm::vec3));
    AddFunction("float dot(const vec3& in, const vec3& in)", WRAP_FN_PR(glm::dot, (const glm::vec3&, const glm::vec3&), float));
    AddFunction("vec3 cross(const vec3& in, const vec3& in)", WRAP_FN_PR(glm::cross, (const glm::vec3&, const glm::vec3&), glm::vec3));

    AddFunction("vec3 reflect(const vec3& in, const vec3& in)", WRAP_FN_PR(glm::reflect, (const glm::vec3&, const glm::vec3&), glm::vec3));
}

void ScriptManager::RegisterVec4() const
{
    AddValueType("vec4", sizeof(glm::vec4), asGetTypeTraits<glm::vec4>() | asOBJ_POD,
        {
            { "vec4 opAdd(const vec4& in)", WRAP_OBJ_FIRST_PR(glm::operator+, (const glm::vec4&, const glm::vec4&), glm::vec4) },
            { "vec4 opSub(const vec4& in)", WRAP_OBJ_FIRST_PR(glm::operator-, (const glm::vec4&, const glm::vec4&), glm::vec4) },
            { "vec4 opMul(const vec4& in)", WRAP_OBJ_FIRST_PR(glm::operator*, (const glm::vec4&, const glm::vec4&), glm::vec4) },
            { "vec4 opDiv(const vec4& in)", WRAP_OBJ_FIRST_PR(glm::operator/, (const glm::vec4&, const glm::vec4&), glm::vec4) },
            { "vec4 opMul(float)", WRAP_OBJ_FIRST_PR(glm::operator*, (const glm::vec4&, float), glm::vec4) },
            { "vec4 opDiv(float)", WRAP_OBJ_FIRST_PR(glm::operator/, (const glm::vec4&, float), glm::vec4) },
            { "vec4 opAddAssign(const vec4& in)", WRAP_OBJ_LAST(as::Vec4AddAssign) },
            { "vec4 opSubAssign(const vec4& in)", WRAP_OBJ_LAST(as::Vec4SubAssign) },
            { "vec4 opMulAssign(const vec4& in)", WRAP_OBJ_LAST(as::Vec4MulAssign) },
            { "vec4 opDivAssign(const vec4& in)", WRAP_OBJ_LAST(as::Vec4DivAssign) },
            { "vec4 opMulAssign(const vec4& in)", WRAP_OBJ_LAST(as::Vec4MulAssignScalar) },
            { "vec4 opDivAssign(const vec4& in)", WRAP_OBJ_LAST(as::Vec4DivAssignScalar) }
        },
        {
            { "float x", asOFFSET(glm::vec4, x) },
            { "float y", asOFFSET(glm::vec4, y) },
            { "float z", asOFFSET(glm::vec4, z) },
            { "float w", asOFFSET(glm::vec4, w) }
        }
    );

    AddTypeConstructor("vec4", "void f(float)", WRAP_OBJ_LAST(as::MakeVec4Scalar));
    AddTypeConstructor("vec4", "void f(float, float, float, float)", WRAP_OBJ_LAST(as::MakeVec4));

    AddFunction("float length(const vec4& in)", WRAP_FN_PR(glm::length, (const glm::vec4&), float));
    AddFunction("vec4 normalize(const vec4& in)", WRAP_FN_PR(glm::normalize, (const glm::vec4&), glm::vec4));
    AddFunction("float dot(const vec4& in, const vec4& in)", WRAP_FN_PR(glm::dot, (const glm::vec4&, const glm::vec4&), float));
}

void ScriptManager::RegisterQuat() const
{
    AddValueType("quat", sizeof(glm::quat), asGetTypeTraits<glm::quat>() | asOBJ_POD,
        {
            { "vec3 opMul(const vec3& in)", WRAP_OBJ_FIRST_PR(glm::operator*, (const glm::quat&, const glm::vec3&), glm::vec3) },
            { "quat opMul(const quat& in)", WRAP_OBJ_FIRST_PR(glm::operator*, (const glm::quat&, const glm::quat&), glm::quat) }
        },
        {
            { "float x", asOFFSET(glm::quat, x) },
            { "float y", asOFFSET(glm::quat, y) },
            { "float z", asOFFSET(glm::quat, z) },
            { "float w", asOFFSET(glm::quat, w) }
        }
    );

    AddTypeConstructor("quat", "void f(float, float, float, float)", WRAP_OBJ_LAST(as::MakeQuat));
    AddTypeConstructor("quat", "void f(const vec3& in)", WRAP_OBJ_LAST(as::MakeQuatFromEuler));
    AddTypeConstructor("quat", "void f(const mat4& in)", WRAP_OBJ_LAST(as::MakeQuatFromMat4));

    AddFunction("quat slerp(const quat& in, const quat& in, float)", WRAP_FN_PR(glm::slerp, (const glm::quat&, const glm::quat&, float), glm::quat));
    AddFunction("quat lerp(const quat& in, const quat& in, float)", WRAP_FN_PR(glm::lerp, (const glm::quat&, const glm::quat&, float), glm::quat));

    AddFunction("vec3 rotate(const quat& in, const vec3& in)", WRAP_FN_PR(glm::rotate, (const glm::quat&, const glm::vec3&), glm::vec3));
    AddFunction("quat normalize(const quat& in)", WRAP_FN_PR(glm::normalize, (const glm::quat&), glm::quat));

    AddFunction("vec3 eulerAngles(const quat& in)", WRAP_FN_PR(glm::eulerAngles, (const glm::quat&), glm::vec3));
    AddFunction("float length(const quat& in)", WRAP_FN_PR(glm::length, (const glm::quat&), float));
    AddFunction("quat conjugate(const quat& in)", WRAP_FN_PR(glm::conjugate, (const glm::quat&), glm::quat));
    AddFunction("quat inverse(const quat& in)", WRAP_FN_PR(glm::inverse, (const glm::quat&), glm::quat));
    AddFunction("float dot(const quat& in, const quat& in)", WRAP_FN_PR(glm::dot, (const glm::quat&, const glm::quat&), float));
}

void ScriptManager::RegisterMat4() const
{
    AddValueType("mat4", sizeof(glm::mat4), asGetTypeTraits<glm::mat4>() | asOBJ_POD,
        {
            { "vec4 opMul(const vec4& in)", WRAP_OBJ_FIRST_PR(glm::operator*, (const glm::mat4&, const glm::vec4&), glm::vec4) },
            { "mat4 opMul(const mat4& in)", WRAP_OBJ_FIRST_PR(glm::operator*, (const glm::mat4&, const glm::mat4&), glm::mat4) }
        },
        {}
    );

    AddTypeConstructor("mat4", "void f()", WRAP_OBJ_LAST(as::MakeType<glm::mat4>));

    AddFunction("mat4 lookAt(const vec3& in, const vec3& in, const vec3& in)", WRAP_FN_PR(glm::lookAt, (const glm::vec3&, const glm::vec3&, const glm::vec3&), glm::mat4));

    AddFunction("mat4 transpose(const mat4& in)", WRAP_FN_PR(glm::transpose, (const glm::mat4&), glm::mat4));
    AddFunction("mat4 inverse(const mat4& in)", WRAP_FN_PR(glm::inverse, (const glm::mat4&), glm::mat4));
}

void ScriptManager::RegisterGLM() const
{
    RegisterVec2();
    RegisterVec3();
    RegisterVec4();
    RegisterMat4();
    RegisterQuat();

    AddFunction("float radians(float)", WRAP_FN_PR(glm::radians, (float), float));
    AddFunction("vec2 radians(const vec2& in)", WRAP_FN_PR(glm::radians, (const glm::vec2&), glm::vec2));
    AddFunction("vec3 radians(const vec3& in)", WRAP_FN_PR(glm::radians, (const glm::vec3&), glm::vec3));
    AddFunction("vec4 radians(const vec4& in)", WRAP_FN_PR(glm::radians, (const glm::vec4&), glm::vec4));

    AddFunction("float degrees(float)", WRAP_FN_PR(glm::degrees, (float), float));
    AddFunction("vec2 degrees(const vec2& in)", WRAP_FN_PR(glm::degrees, (const glm::vec2&), glm::vec2));
    AddFunction("vec3 degrees(const vec3& in)", WRAP_FN_PR(glm::degrees, (const glm::vec3&), glm::vec3));
    AddFunction("vec4 degrees(const vec4& in)", WRAP_FN_PR(glm::degrees, (const glm::vec4&), glm::vec4));

    AddFunction("float mix(float, float, float)", WRAP_FN_PR(glm::mix, (float, float, float), float));
    AddFunction("vec2 mix(const vec2& in, const vec2& in, float)", WRAP_FN_PR(glm::mix, (const glm::vec2&, const glm::vec2&, float), glm::vec2));
    AddFunction("vec3 mix(const vec3& in, const vec3& in, float)", WRAP_FN_PR(glm::mix, (const glm::vec3&, const glm::vec3&, float), glm::vec3));
    AddFunction("vec4 mix(const vec4& in, const vec4& in, float)", WRAP_FN_PR(glm::mix, (const glm::vec4&, const glm::vec4&, float), glm::vec4));

    AddFunction("quat mix(const quat& in, const quat& in, float)", WRAP_FN_PR(glm::mix, (const glm::quat&, const glm::quat&, float), glm::quat));

    AddFunction("float clamp(float, float, float)", WRAP_FN_PR(glm::clamp, (float, float, float), float));
    AddFunction("vec2 clamp(const vec2& in, const vec2& in, const vec2& in)", WRAP_FN_PR(glm::clamp, (const glm::vec2&, const glm::vec2&, const glm::vec2&), glm::vec2));
    AddFunction("vec3 clamp(const vec3& in, const vec3& in, const vec3& in)", WRAP_FN_PR(glm::clamp, (const glm::vec3&, const glm::vec3&, const glm::vec3&), glm::vec3));
    AddFunction("vec4 clamp(const vec4& in, const vec4& in, const vec4& in)", WRAP_FN_PR(glm::clamp, (const glm::vec4&, const glm::vec4&, const glm::vec4&), glm::vec4));

    AddFunction("float fract(float)", WRAP_FN_PR(glm::fract, (float), float));
    AddFunction("vec2 fract(const vec2& in)", WRAP_FN_PR(glm::fract, (const glm::vec2&), glm::vec2));
    AddFunction("vec3 fract(const vec3& in)", WRAP_FN_PR(glm::fract, (const glm::vec3&), glm::vec3));

    AddFunction("bool isnan(float)", WRAP_FN_PR(std::isnan, (float), bool));
    AddFunction("bool isinf(float)", WRAP_FN_PR(std::isinf, (float), bool));
}

void ScriptManager::RegisterBody() const
{
    AddValueType("BodyID", sizeof(JPH::BodyID), asGetTypeTraits<JPH::BodyID>() | asOBJ_POD, {}, {});

    AddType("Body", sizeof(JPH::Body),
        {
            { "const BodyID& GetID() const", WRAP_MFN(JPH::Body, GetID) },
            { "glm::vec3 GetPosition() const", WRAP_OBJ_LAST(as::GetPosition) },
            { "glm::quat GetRotation() const", WRAP_OBJ_LAST(as::GetRotation) },
            { "void SetPosition(const glm::vec3& in)", WRAP_OBJ_LAST(as::SetPosition) },
            { "void SetRotation(const glm::quat& in)", WRAP_OBJ_LAST(as::SetRotation) },
            { "void SetPositionAndRotation(const glm::vec3& in, const glm::quat& in)", WRAP_OBJ_LAST(as::SetPositionAndRotation) },
            { "void AddForce(const glm::vec3& in)", WRAP_OBJ_LAST(as::AddForce) },
            { "void AddTorque(const glm::vec3& in)", WRAP_OBJ_LAST(as::AddTorque) },
            { "void AddImpulse(const glm::vec3& in)", WRAP_OBJ_LAST(as::AddImpulse) },
            { "void SetLinearVelocity(const glm::vec3& in)", WRAP_OBJ_LAST(as::SetLinearVelocity) },
            { "void SetAngularVelocity(const glm::vec3& in)", WRAP_OBJ_LAST(as::SetAngularVelocity) },
            { "void SetIsSensor(bool)", WRAP_MFN(JPH::Body, SetIsSensor) },
            { "void SetFriction(float)", WRAP_MFN(JPH::Body, SetFriction) },
            { "void SetGravityFactor(float)", WRAP_OBJ_LAST(as::SetGravityFactor) },
            { "void SetMass(float)", WRAP_OBJ_LAST(as::SetMass) },
            { "glm::vec3 GetLinearVelocity() const", WRAP_OBJ_LAST(as::GetLinearVelocity) },
            { "glm::vec3 GetAngularVelocity() const", WRAP_OBJ_LAST(as::GetAngularVelocity) },
            { "bool IsSensor()", WRAP_MFN(JPH::Body, IsSensor) },
            { "float GetFriction()", WRAP_MFN(JPH::Body, GetFriction) }
        }, {}
    );
}

void ScriptManager::RegisterRayCast() const
{
    AddValueType("RayCastResult", sizeof(as::RayCastResult), asGetTypeTraits<as::RayCastResult>() | asOBJ_POD, {},
        {
            { "bool hit", asOFFSET(as::RayCastResult, hit) },
            { "glm::vec3 hitPosition", asOFFSET(as::RayCastResult, hitPosition) },
            { "Body@ body", asOFFSET(as::RayCastResult, body) }
        }
    );

    AddFunction("RayCastResult CastRay(const glm::vec3& in, const glm::vec3& in)", WRAP_FN(as::CastRay));
}

void ScriptManager::RegisterExtent2D() const
{
    AddValueType("Extent2D", sizeof(LLGL::Extent2D), asGetTypeTraits<LLGL::Extent2D>() | asOBJ_POD, {},
        {
            { "uint32 width", asOFFSET(LLGL::Extent2D, width) },
            { "uint32 height", asOFFSET(LLGL::Extent2D, height) }
        }
    );

    AddTypeConstructor("Extent2D", "void f(uint32, uint32)", WRAP_OBJ_LAST(as::MakeExtent2D));
}

void ScriptManager::RegisterCamera() const
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
            { "void SetUp(const glm::vec3& in)", WRAP_MFN(Camera, SetUp) },
            { "void SetLookAt(const glm::vec3& in)", WRAP_MFN(Camera, SetLookAt) },
            { "void SetViewMatrix(const glm::mat4& in)", WRAP_MFN(Camera, SetViewMatrix) },

            { "glm::vec2 WorldToScreen(const glm::vec3& in) const", WRAP_MFN(Camera, WorldToScreen) },
            { "glm::vec3 ScreenToWorld(const glm::vec2& in) const", WRAP_MFN(Camera, ScreenToWorld) },

            { "glm::vec3 GetUp() const", WRAP_MFN(Camera, GetUp) },
            { "glm::vec3 GetLookAt() const", WRAP_MFN(Camera, GetLookAt) },
            { "glm::mat4 GetViewMatrix() const", WRAP_MFN(Camera, GetViewMatrix) },
            { "glm::mat4 GetProjectionMatrix() const", WRAP_MFN(Camera, GetProjectionMatrix) },
            { "glm::vec2 GetViewport() const", WRAP_MFN(Camera, GetViewport) },

            { "float GetFov() const", WRAP_MFN(Camera, GetFov) },
            { "float GetNear() const", WRAP_MFN(Camera, GetNear) },
            { "float GetFar() const", WRAP_MFN(Camera, GetFar) },

            { "float GetAspect() const", WRAP_MFN(Camera, GetAspect) },

            { "bool IsFirstPerson() const", WRAP_MFN(Camera, IsFirstPerson) }
        }, {}
    );
}

void ScriptManager::RegisterSound() const
{
    AddValueType("Sound", sizeof(Sound), asGetTypeTraits<Sound>() | asOBJ_POD,
        {
            { "void Play()", WRAP_MFN(Sound, Play) },
            { "void Stop()", WRAP_MFN(Sound, Stop) },

            { "void SetPosition(const glm::vec3& in)", WRAP_MFN(Sound, SetPosition) },
            { "void SetVelocity(const glm::vec3& in)", WRAP_MFN(Sound, SetVelocity) },
            { "void SetOrientation(const glm::quat& in)", WRAP_MFN(Sound, SetOrientation) },
            { "void SetDirection(const glm::vec3& in)", WRAP_MFN(Sound, SetDirection) },
            // { "void SetCone(const Sound::Cone& in)", WRAP_MFN(Sound, SetCone) },

            { "void SetDirectionalAttenuationFactor(float)", WRAP_MFN(Sound, SetDirectionalAttenuationFactor) },
            { "void SetDopplerFactor(float)", WRAP_MFN(Sound, SetDopplerFactor) },
            { "void SetFade(float, float, uint64)", WRAP_MFN(Sound, SetFade) },
            { "void SetRolloff(float)", WRAP_MFN(Sound, SetRolloff) },

            { "void SetStartTime(uint64)", WRAP_MFN(Sound, SetStartTime) },
            { "void SetStopTime(uint64)", WRAP_MFN(Sound, SetStopTime) },

            { "void SetMaxDistance(float)", WRAP_MFN(Sound, SetMaxDistance) },
            { "void SetMinDistance(float)", WRAP_MFN(Sound, SetMinDistance) },
            { "void SetMaxGain(float)", WRAP_MFN(Sound, SetMaxGain) },
            { "void SetMinGain(float)", WRAP_MFN(Sound, SetMinGain) },

            { "void SetPan(float)", WRAP_MFN(Sound, SetPan) },
            { "void SetPitch(float)", WRAP_MFN(Sound, SetPitch) },
            { "void SetVolume(float)", WRAP_MFN(Sound, SetVolume) },

            { "void SetLooping(bool)", WRAP_MFN(Sound, SetLooping) },
            { "void SetSpatializationEnabled(bool)", WRAP_MFN(Sound, SetSpatializationEnabled) }
        }, {}
    );
}

void ScriptManager::RegisterKeyboard() const
{
    AddFunction("bool IsKeyPressed(int)", WRAP_FN(Keyboard::IsKeyPressed));
    AddFunction("bool IsKeyReleased(int)", WRAP_FN(Keyboard::IsKeyReleased));
    AddFunction("bool IsKeyRepeated(int)", WRAP_FN(Keyboard::IsKeyRepeated));

    AddEnumValues("Key",
        {
            { "Unknown", static_cast<int>(Keyboard::Key::Unknown) },
            { "Space", static_cast<int>(Keyboard::Key::Space) },
            { "Apostrophe", static_cast<int>(Keyboard::Key::Apostrophe) },
            { "Comma", static_cast<int>(Keyboard::Key::Comma) },
            { "Minus", static_cast<int>(Keyboard::Key::Minus) },
            { "Period", static_cast<int>(Keyboard::Key::Period) },
            { "Slash", static_cast<int>(Keyboard::Key::Slash) },
            { "Num0", static_cast<int>(Keyboard::Key::Num0) },
            { "Num1", static_cast<int>(Keyboard::Key::Num1) },
            { "Num2", static_cast<int>(Keyboard::Key::Num2) },
            { "Num3", static_cast<int>(Keyboard::Key::Num3) },
            { "Num4", static_cast<int>(Keyboard::Key::Num4) },
            { "Num5", static_cast<int>(Keyboard::Key::Num5) },
            { "Num6", static_cast<int>(Keyboard::Key::Num6) },
            { "Num7", static_cast<int>(Keyboard::Key::Num7) },
            { "Num8", static_cast<int>(Keyboard::Key::Num8) },
            { "Num9", static_cast<int>(Keyboard::Key::Num9) },
            { "Semicolon", static_cast<int>(Keyboard::Key::Semicolon) },
            { "Equal", static_cast<int>(Keyboard::Key::Equal) },
            { "A", static_cast<int>(Keyboard::Key::A) },
            { "B", static_cast<int>(Keyboard::Key::B) },
            { "C", static_cast<int>(Keyboard::Key::C) },
            { "D", static_cast<int>(Keyboard::Key::D) },
            { "E", static_cast<int>(Keyboard::Key::E) },
            { "F", static_cast<int>(Keyboard::Key::F) },
            { "G", static_cast<int>(Keyboard::Key::G) },
            { "H", static_cast<int>(Keyboard::Key::H) },
            { "I", static_cast<int>(Keyboard::Key::I) },
            { "J", static_cast<int>(Keyboard::Key::J) },
            { "K", static_cast<int>(Keyboard::Key::K) },
            { "L", static_cast<int>(Keyboard::Key::L) },
            { "M", static_cast<int>(Keyboard::Key::M) },
            { "N", static_cast<int>(Keyboard::Key::N) },
            { "O", static_cast<int>(Keyboard::Key::O) },
            { "P", static_cast<int>(Keyboard::Key::P) },
            { "Q", static_cast<int>(Keyboard::Key::Q) },
            { "R", static_cast<int>(Keyboard::Key::R) },
            { "S", static_cast<int>(Keyboard::Key::S) },
            { "T", static_cast<int>(Keyboard::Key::T) },
            { "U", static_cast<int>(Keyboard::Key::U) },
            { "V", static_cast<int>(Keyboard::Key::V) },
            { "W", static_cast<int>(Keyboard::Key::W) },
            { "X", static_cast<int>(Keyboard::Key::X) },
            { "Y", static_cast<int>(Keyboard::Key::Y) },
            { "Z", static_cast<int>(Keyboard::Key::Z) },
            { "LeftBracket", static_cast<int>(Keyboard::Key::LeftBracket) },
            { "Backslash", static_cast<int>(Keyboard::Key::Backslash) },
            { "RightBracket", static_cast<int>(Keyboard::Key::RightBracket) },
            { "GraveAccent", static_cast<int>(Keyboard::Key::GraveAccent) },
            { "World1", static_cast<int>(Keyboard::Key::World1) },
            { "World2", static_cast<int>(Keyboard::Key::World2) },
            { "Escape", static_cast<int>(Keyboard::Key::Escape) },
            { "Enter", static_cast<int>(Keyboard::Key::Enter) },
            { "Tab", static_cast<int>(Keyboard::Key::Tab) },
            { "Backspace", static_cast<int>(Keyboard::Key::Backspace) },
            { "Insert", static_cast<int>(Keyboard::Key::Insert) },
            { "Delete", static_cast<int>(Keyboard::Key::Delete) },
            { "Right", static_cast<int>(Keyboard::Key::Right) },
            { "Left", static_cast<int>(Keyboard::Key::Left) },
            { "Down", static_cast<int>(Keyboard::Key::Down) },
            { "Up", static_cast<int>(Keyboard::Key::Up) },
            { "PageUp", static_cast<int>(Keyboard::Key::PageUp) },
            { "PageDown", static_cast<int>(Keyboard::Key::PageDown) },
            { "Home", static_cast<int>(Keyboard::Key::Home) },
            { "End", static_cast<int>(Keyboard::Key::End) },
            { "CapsLock", static_cast<int>(Keyboard::Key::CapsLock) },
            { "ScrollLock", static_cast<int>(Keyboard::Key::ScrollLock) },
            { "NumLock", static_cast<int>(Keyboard::Key::NumLock) },
            { "PrintScreen", static_cast<int>(Keyboard::Key::PrintScreen) },
            { "Pause", static_cast<int>(Keyboard::Key::Pause) },
            { "F1", static_cast<int>(Keyboard::Key::F1) },
            { "F2", static_cast<int>(Keyboard::Key::F2) },
            { "F3", static_cast<int>(Keyboard::Key::F3) },
            { "F4", static_cast<int>(Keyboard::Key::F4) },
            { "F5", static_cast<int>(Keyboard::Key::F5) },
            { "F6", static_cast<int>(Keyboard::Key::F6) },
            { "F7", static_cast<int>(Keyboard::Key::F7) },
            { "F8", static_cast<int>(Keyboard::Key::F8) },
            { "F9", static_cast<int>(Keyboard::Key::F9) },
            { "F10", static_cast<int>(Keyboard::Key::F10) },
            { "F11", static_cast<int>(Keyboard::Key::F11) },
            { "F12", static_cast<int>(Keyboard::Key::F12) },
            { "F13", static_cast<int>(Keyboard::Key::F13) },
            { "F14", static_cast<int>(Keyboard::Key::F14) },
            { "F15", static_cast<int>(Keyboard::Key::F15) },
            { "F16", static_cast<int>(Keyboard::Key::F16) },
            { "F17", static_cast<int>(Keyboard::Key::F17) },
            { "F18", static_cast<int>(Keyboard::Key::F18) },
            { "F19", static_cast<int>(Keyboard::Key::F19) },
            { "F20", static_cast<int>(Keyboard::Key::F20) },
            { "F21", static_cast<int>(Keyboard::Key::F21) },
            { "F22", static_cast<int>(Keyboard::Key::F22) },
            { "F23", static_cast<int>(Keyboard::Key::F23) },
            { "F24", static_cast<int>(Keyboard::Key::F24) },
            { "F25", static_cast<int>(Keyboard::Key::F25) },
            { "Kp0", static_cast<int>(Keyboard::Key::Kp0) },
            { "Kp1", static_cast<int>(Keyboard::Key::Kp1) },
            { "Kp2", static_cast<int>(Keyboard::Key::Kp2) },
            { "Kp3", static_cast<int>(Keyboard::Key::Kp3) },
            { "Kp4", static_cast<int>(Keyboard::Key::Kp4) },
            { "Kp5", static_cast<int>(Keyboard::Key::Kp5) },
            { "Kp6", static_cast<int>(Keyboard::Key::Kp6) },
            { "Kp7", static_cast<int>(Keyboard::Key::Kp7) },
            { "Kp8", static_cast<int>(Keyboard::Key::Kp8) },
            { "Kp9", static_cast<int>(Keyboard::Key::Kp9) },
            { "KpDecimal", static_cast<int>(Keyboard::Key::KpDecimal) },
            { "KpDivide", static_cast<int>(Keyboard::Key::KpDivide) },
            { "KpMultiply", static_cast<int>(Keyboard::Key::KpMultiply) },
            { "KpSubtract", static_cast<int>(Keyboard::Key::KpSubtract) },
            { "KpAdd", static_cast<int>(Keyboard::Key::KpAdd) },
            { "KpEnter", static_cast<int>(Keyboard::Key::KpEnter) },
            { "KpEqual", static_cast<int>(Keyboard::Key::KpEqual) },
            { "LeftShift", static_cast<int>(Keyboard::Key::LeftShift) },
            { "LeftControl", static_cast<int>(Keyboard::Key::LeftControl) },
            { "LeftAlt", static_cast<int>(Keyboard::Key::LeftAlt) },
            { "LeftSuper", static_cast<int>(Keyboard::Key::LeftSuper) },
            { "RightShift", static_cast<int>(Keyboard::Key::RightShift) },
            { "RightControl", static_cast<int>(Keyboard::Key::RightControl) },
            { "RightAlt", static_cast<int>(Keyboard::Key::RightAlt) },
            { "RightSuper", static_cast<int>(Keyboard::Key::RightSuper) },
            { "Menu", static_cast<int>(Keyboard::Key::Menu) },
            { "Last", static_cast<int>(Keyboard::Key::Last) }
        }
    );
}

void ScriptManager::RegisterMouse() const
{
    AddFunction("glm::vec2 GetPosition()", WRAP_FN(Mouse::GetPosition));
    AddFunction("void SetPosition(const glm::vec2& in)", WRAP_FN(Mouse::SetPosition));
    AddFunction("bool IsButtonPressed(int)", WRAP_FN(Mouse::IsButtonPressed));
    AddFunction("bool IsButtonReleased(int)", WRAP_FN(Mouse::IsButtonReleased));
    AddFunction("void SetCursorVisible(bool = true)", WRAP_FN(Mouse::SetCursorVisible));

    AddEnumValues("Button",
        {
            { "Left", static_cast<int>(Mouse::Button::Left) },
            { "Right", static_cast<int>(Mouse::Button::Right) },
            { "Middle", static_cast<int>(Mouse::Button::Middle) },
            { "MButton4", static_cast<int>(Mouse::Button::MButton4) },
            { "MButton5", static_cast<int>(Mouse::Button::MButton5) },
            { "MButton6", static_cast<int>(Mouse::Button::MButton6) },
            { "MButton7", static_cast<int>(Mouse::Button::MButton7) },
            { "MButton8", static_cast<int>(Mouse::Button::MButton8) },
            { "Last", static_cast<int>(Mouse::Button::Last) }
        }
    );
}

void ScriptManager::RegisterInputManager() const
{
    AddFunction("void MapKeyboardAction(const string& in, int)", WRAP_FN_PR(as::MapAction, (const std::string&, Keyboard::Key), void));
    AddFunction("void MapMouseAction(const string& in, int)", WRAP_FN_PR(as::MapAction, (const std::string&, Mouse::Button), void));
    AddFunction("bool IsActionPressed(const string& in)", WRAP_FN(as::IsActionPressed));
}

void ScriptManager::RegisterScriptManager() const
{
    AddFunction("void AddScript(ScriptAssetPtr)", WRAP_MFN(ScriptManager, AddScript));
    AddFunction("void RemoveScript(ScriptAssetPtr)", WRAP_MFN(ScriptManager, RemoveScript));
    AddFunction("void Build()", WRAP_MFN(ScriptManager, Build));
    AddFunction("void ExecuteFunction(ScriptAssetPtr, const string& in, uint32 = 0)", WRAP_OBJ_LAST(as::ExecuteFunction));
}

void ScriptManager::RegisterTextureAsset() const
{
    AddType("TextureAsset", sizeof(TextureAsset), {}, {});

    AddValueType("TextureAssetPtr", sizeof(TextureAssetPtr), asGetTypeTraits<TextureAssetPtr>() | asOBJ_POD,
        {
            { "TextureAsset@ get()", WRAP_OBJ_LAST(as::GetAssetPtr<TextureAsset>) },
            { "TextureAssetPtr opAssign(const TextureAssetPtr& in)", WRAP_MFN_PR(TextureAssetPtr, operator=, (const TextureAssetPtr&), TextureAssetPtr&) }
        }, {}
    );

    AddTypeConstructor("TextureAssetPtr", "void f(const TextureAssetPtr& in)", WRAP_OBJ_LAST(as::CopyType<TextureAssetPtr>));
}

void ScriptManager::RegisterMaterialAsset() const
{
    AddValueType("MaterialProperty", sizeof(MaterialAsset::Property), asGetTypeTraits<MaterialAsset::Property>() | asOBJ_POD,
        {
            { "MaterialProperty& opAssign(const glm::vec4& in)", WRAP_MFN_PR(MaterialAsset::Property, operator=, (const glm::vec4&), MaterialAsset::Property&) },
            { "MaterialProperty& opAssign(const TextureAssetPtr& in)", WRAP_MFN_PR(MaterialAsset::Property, operator=, (const TextureAssetPtr&), MaterialAsset::Property&) },
        },
        {
            { "int type", asOFFSET(MaterialAsset::Property, type) },
            { "glm::vec4 value", asOFFSET(MaterialAsset::Property, value) },
            { "TextureAssetPtr texture", asOFFSET(MaterialAsset::Property, texture) }
        }
    );

    AddEnum("MaterialPropertyType", { "PropertyTexture", "PropertyColor" });

    AddType("MaterialAsset", sizeof(MaterialAsset), {},
        {
            { "MaterialProperty albedo", asOFFSET(MaterialAsset, albedo) },
            { "MaterialProperty normal", asOFFSET(MaterialAsset, normal) },
            { "MaterialProperty metallic", asOFFSET(MaterialAsset, metallic) },
            { "MaterialProperty roughness", asOFFSET(MaterialAsset, roughness) },
            { "MaterialProperty ao", asOFFSET(MaterialAsset, ao) },
            { "MaterialProperty emission", asOFFSET(MaterialAsset, emission) },
            { "float emissionStrength", asOFFSET(MaterialAsset, emissionStrength) },
            { "glm::vec2 uvScale", asOFFSET(MaterialAsset, uvScale) }
        }
    );

    AddValueType("MaterialAssetPtr", sizeof(MaterialAssetPtr), asGetTypeTraits<MaterialAssetPtr>() | asOBJ_POD,
        {
            { "MaterialAsset@ get()", WRAP_OBJ_LAST(as::GetAssetPtr<MaterialAsset>) },
            { "MaterialAssetPtr opAssign(const MaterialAssetPtr& in)", WRAP_MFN_PR(MaterialAssetPtr, operator=, (const MaterialAssetPtr&), MaterialAssetPtr&) }
        }, {}
    );

    AddTypeConstructor("MaterialAssetPtr", "void f(const MaterialAssetPtr& in)", WRAP_OBJ_LAST(as::CopyType<MaterialAssetPtr>));
}

void ScriptManager::RegisterModelAsset() const
{
    AddType("ModelAsset", sizeof(ModelAsset), {}, {});

    AddValueType("ModelAssetPtr", sizeof(ModelAssetPtr), asGetTypeTraits<ModelAssetPtr>() | asOBJ_POD,
        {
            { "ModelAsset@ get()", WRAP_OBJ_LAST(as::GetAssetPtr<ModelAsset>) },
            { "ModelAssetPtr opAssign(const ModelAssetPtr& in)", WRAP_MFN_PR(ModelAssetPtr, operator=, (const ModelAssetPtr&), ModelAssetPtr&) }
        }, {}
    );

    AddTypeConstructor("ModelAssetPtr", "void f(const ModelAssetPtr& in)", WRAP_OBJ_LAST(as::CopyType<ModelAssetPtr>));
}

void ScriptManager::RegisterSceneAsset() const
{
    AddType("SceneAsset", sizeof(SceneAsset), {},
        {
            { "Scene@ scene", asOFFSET(SceneAsset, scene) }
        }
    );

    AddValueType("SceneAssetPtr", sizeof(SceneAssetPtr), asGetTypeTraits<SceneAssetPtr>() | asOBJ_POD,
        {
            { "SceneAsset@ get()", WRAP_OBJ_LAST(as::GetAssetPtr<SceneAsset>) },
            { "SceneAssetPtr opAssign(const SceneAssetPtr& in)", WRAP_MFN_PR(SceneAssetPtr, operator=, (const SceneAssetPtr&), SceneAssetPtr&) }
        }, {}
    );

    AddTypeConstructor("SceneAssetPtr", "void f(const SceneAssetPtr& in)", WRAP_OBJ_LAST(as::CopyType<SceneAssetPtr>));
}

void ScriptManager::RegisterScriptAsset() const
{
    AddType("ScriptAsset", sizeof(ScriptAsset), {}, {});

    AddValueType("ScriptAssetPtr", sizeof(ScriptAssetPtr), asGetTypeTraits<ScriptAssetPtr>() | asOBJ_POD,
        {
            { "ScriptAsset@ get()", WRAP_OBJ_LAST(as::GetAssetPtr<ScriptAsset>) },
            { "ScriptAssetPtr opAssign(const ScriptAssetPtr& in)", WRAP_MFN_PR(ScriptAssetPtr, operator=, (const ScriptAssetPtr&), ScriptAssetPtr&) }
        }, {}
    );

    AddTypeConstructor("ScriptAssetPtr", "void f(const ScriptAssetPtr& in)", WRAP_OBJ_LAST(as::CopyType<ScriptAssetPtr>));
}

void ScriptManager::RegisterSoundAsset() const
{
    AddType("SoundAsset", sizeof(SoundAsset), {},
        {
            { "Sound sound", asOFFSET(SoundAsset, sound) }
        }
    );

    AddValueType("SoundAssetPtr", sizeof(SoundAssetPtr), asGetTypeTraits<SoundAssetPtr>() | asOBJ_POD,
        {
            { "SoundAsset@ get()", WRAP_OBJ_LAST(as::GetAssetPtr<SoundAsset>) },
            { "SoundAssetPtr opAssign(const SoundAssetPtr& in)", WRAP_MFN_PR(SoundAssetPtr, operator=, (const SoundAssetPtr&), SoundAssetPtr&) }
        }, {}
    );

    AddTypeConstructor("SoundAssetPtr", "void f(const SoundAssetPtr& in)", WRAP_OBJ_LAST(as::CopyType<SoundAssetPtr>));
}

void ScriptManager::RegisterAssetManager() const
{
    AddFunction("TextureAssetPtr LoadTexture(const string& in, bool = false, bool = true, bool = true)", WRAP_FN(as::Load<TextureAsset>));
    AddFunction("MaterialAssetPtr LoadMaterial(const string& in, bool = false, bool = true, bool = true)", WRAP_FN(as::Load<MaterialAsset>));
    AddFunction("ModelAssetPtr LoadModel(const string& in, bool = false, bool = true, bool = true)", WRAP_FN(as::Load<ModelAsset>));
    AddFunction("SoundAssetPtr LoadSound(const string& in, bool = false, bool = true, bool = true)", WRAP_FN(as::Load<SoundAsset>));
}

void ScriptManager::RegisterWindowResizeEvent() const
{
    AddType("WindowResizeEvent", sizeof(WindowResizeEvent),
        {
            { "void SetHandled()", WRAP_MFN(WindowResizeEvent, SetHandled) },
            { "bool IsHandled() const", WRAP_MFN(WindowResizeEvent, IsHandled) },
            { "Extent2D GetSize() const", WRAP_MFN(WindowResizeEvent, GetSize) }
        }, {}
    );
}

void ScriptManager::RegisterAssetLoadedEvent()
{
    // TODO ...
}

void ScriptManager::RegisterCollisionEvent() const
{
    AddType("CollisionEvent", sizeof(CollisionEvent),
        {
            { "void SetHandled()", WRAP_MFN(CollisionEvent, SetHandled) },
            { "bool IsHandled() const", WRAP_MFN(CollisionEvent, IsHandled) },
            { "JPH::Body@ GetBody1() const", WRAP_MFN(CollisionEvent, GetBody1) },
            { "JPH::Body@ GetBody2() const", WRAP_MFN(CollisionEvent, GetBody2) },
            { "glm::vec3 GetContactPosition1() const", WRAP_MFN(CollisionEvent, GetContactPosition1) },
            { "glm::vec3 GetContactPosition2() const", WRAP_MFN(CollisionEvent, GetContactPosition2) },
            { "glm::vec3 GetNormal() const", WRAP_MFN(CollisionEvent, GetNormal) },
            { "float GetPenetration() const", WRAP_MFN(CollisionEvent, GetPenetration) }
        }, {}
    );
}

void ScriptManager::RegisterTimer() const
{
    AddValueType("Timer", sizeof(Timer), asGetTypeTraits<Timer>() | asOBJ_POD,
        {
            { "void Reset()", WRAP_MFN(Timer, Reset) },
            { "float GetElapsedSeconds()", WRAP_MFN(Timer, GetElapsedSeconds) },
            { "float GetElapsedMilliseconds()", WRAP_MFN(Timer, GetElapsedMilliseconds) }
        }, {}
    );
}

void ScriptManager::RegisterNameComponent() const
{
    AddType("NameComponent", sizeof(NameComponent), {},
        {
            { "string name", asOFFSET(NameComponent, name) }
        }
    );
}

void ScriptManager::RegisterTransformComponent() const
{
    AddType("TransformComponent", sizeof(TransformComponent),
        {
            { "glm::mat4 GetTransform() const", WRAP_MFN(TransformComponent, GetTransform) },
            { "void SetTransform(const glm::mat4& in)", WRAP_MFN(TransformComponent, SetTransform) }
        },
        {
            { "glm::vec3 position", asOFFSET(TransformComponent, position) },
            { "glm::vec3 rotation", asOFFSET(TransformComponent, rotation) },
            { "glm::vec3 scale", asOFFSET(TransformComponent, scale) },
            { "bool overridePhysics", asOFFSET(TransformComponent, overridePhysics) }
        }
    );
}

void ScriptManager::RegisterMeshComponent() const
{
    AddType("MeshComponent", sizeof(MeshComponent), {},
        {
            { "ModelAssetPtr model", asOFFSET(MeshComponent, model) }
        }
    );
}

void ScriptManager::RegisterMeshRendererComponent() const
{
    AddType("MeshRendererComponent", sizeof(MeshRendererComponent),
        {
            { "MaterialAssetPtr& at(uint64)", WRAP_OBJ_LAST(as::MaterialListAt) }
        }, {}
    );
}

void ScriptManager::RegisterCameraComponent() const
{
    AddType("CameraComponent", sizeof(CameraComponent),
        {
            { "Camera@ get_camera() property", WRAP_OBJ_LAST(as::GetCamera) }
        }, {}
    );
}

void ScriptManager::RegisterLightComponent() const
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

void ScriptManager::RegisterScriptComponent() const
{
    AddType("ScriptComponent", sizeof(ScriptComponent), {},
        {
            { "ScriptAssetPtr script", asOFFSET(ScriptComponent, script) },
            { "uint32 moduleIndex", asOFFSET(ScriptComponent, moduleIndex) }
        }
    );
}

void ScriptManager::RegisterBodyComponent() const
{
    AddType("RigidBodyComponent", sizeof(RigidBodyComponent), {},
        {
            { "JPH::Body@ body", asOFFSET(RigidBodyComponent, body) }
        }
    );
}

void ScriptManager::RegisterSoundComponent() const
{
    AddType("SoundComponent", sizeof(SoundComponent), {},
        {
            { "SoundAssetPtr sound", asOFFSET(SoundComponent, sound) }
        }
    );
}

void ScriptManager::RegisterProceduralSkyComponent() const
{
    AddType("ProceduralSkyComponent", sizeof(ProceduralSkyComponent),
        {
            { "void Build()", WRAP_MFN(ProceduralSkyComponent, Build) }
        },
        {
            { "float time", asOFFSET(ProceduralSkyComponent, time) },
            { "float cirrus", asOFFSET(ProceduralSkyComponent, cirrus) },
            { "float cumulus", asOFFSET(ProceduralSkyComponent, cumulus) },
            { "int flip", asOFFSET(ProceduralSkyComponent, flip) }
        }
    );
}

void ScriptManager::RegisterHDRISkyComponent() const
{
    AddType("HDRISkyComponent", sizeof(HDRISkyComponent),
        {
            { "void Build()", WRAP_MFN(HDRISkyComponent, Build) }
        },
        {
            { "TextureAssetPtr environmentMap", asOFFSET(HDRISkyComponent, environmentMap) },
            { "Extent2D resolution", asOFFSET(HDRISkyComponent, resolution) }
        }
    );
}

void ScriptManager::RegisterTonemapComponent() const
{
    AddType("TonemapComponent", sizeof(TonemapComponent), {},
        {
            { "int algorithm", asOFFSET(TonemapComponent, algorithm) },
            { "float exposure", asOFFSET(TonemapComponent, exposure) },
            { "glm::vec3 colorGrading", asOFFSET(TonemapComponent, colorGrading) },
            { "float colorGradingIntensity", asOFFSET(TonemapComponent, colorGradingIntensity) },
            { "float vignetteIntensity", asOFFSET(TonemapComponent, vignetteIntensity) },
            { "float vignetteRoundness", asOFFSET(TonemapComponent, vignetteRoundness) },
            { "float filmGrain", asOFFSET(TonemapComponent, filmGrain) },
            { "float contrast", asOFFSET(TonemapComponent, contrast) },
            { "float saturation", asOFFSET(TonemapComponent, saturation) },
            { "float brightness", asOFFSET(TonemapComponent, brightness) },
            { "TextureAssetPtr lut", asOFFSET(TonemapComponent, lut) }
        }
    );
}

void ScriptManager::RegisterBloomComponent() const
{
    AddType("BloomComponent", sizeof(BloomComponent),
        {
            { "void SetupPostProcessing()", WRAP_MFN(BloomComponent, SetupPostProcessing) }
        },
        {
            { "float threshold", asOFFSET(BloomComponent, threshold) },
            { "float strength", asOFFSET(BloomComponent, strength) },
            { "float resolutionScale", asOFFSET(BloomComponent, resolutionScale) }
        }
    );
}

void ScriptManager::RegisterGTAOComponent() const
{
    AddType("GTAOComponent", sizeof(GTAOComponent),
        {
            { "void SetupPostProcessing()", WRAP_MFN(GTAOComponent, SetupPostProcessing) }
        },
        {
            { "float resolutionScale", asOFFSET(GTAOComponent, resolutionScale) }
        }
    );
}

void ScriptManager::RegisterSSRComponent() const
{
    AddType("SSRComponent", sizeof(SSRComponent),
        {
            { "void SetupPostProcessing()", WRAP_MFN(SSRComponent, SetupPostProcessing) }
        },
        {
            { "float resolutionScale", asOFFSET(SSRComponent, resolutionScale) },
            { "int maxSteps", asOFFSET(SSRComponent, maxSteps) },
            { "int maxBinarySearchSteps", asOFFSET(SSRComponent, maxBinarySearchSteps) },
            { "float rayStep", asOFFSET(SSRComponent, rayStep) }
        }
    );
}

void ScriptManager::RegisterEntity() const
{
    // It will be updated as soon as the Angelscript developer writes some docs on function templates
    AddValueType("Entity", sizeof(Entity), asGetTypeTraits<Entity>() | asOBJ_POD,
        {
            { "NameComponent@ GetNameComponent()", WRAP_MFN(Entity, GetComponent<NameComponent>) },
            { "TransformComponent@ GetTransformComponent()", WRAP_MFN(Entity, GetComponent<TransformComponent>) },
            { "MeshComponent@ GetMeshComponent()", WRAP_MFN(Entity, GetComponent<MeshComponent>) },
            { "MeshRendererComponent@ GetMeshRendererComponent()", WRAP_MFN(Entity, GetComponent<MeshRendererComponent>) },
            { "LightComponent@ GetLightComponent()", WRAP_MFN(Entity, GetComponent<LightComponent>) },
            { "ScriptComponent@ GetScriptComponent()", WRAP_MFN(Entity, GetComponent<ScriptComponent>) },
            { "CameraComponent@ GetCameraComponent()", WRAP_MFN(Entity, GetComponent<CameraComponent>) },
            { "RigidBodyComponent@ GetRigidBodyComponent()", WRAP_MFN(Entity, GetComponent<RigidBodyComponent>) },
            { "SoundComponent@ GetSoundComponent()", WRAP_MFN(Entity, GetComponent<SoundComponent>) },

            { "ProceduralSkyComponent@ GetProceduralSkyComponent()", WRAP_MFN(Entity, GetComponent<ProceduralSkyComponent>) },
            { "HDRISkyComponent@ GetHDRISkyComponent()", WRAP_MFN(Entity, GetComponent<HDRISkyComponent>) },

            { "TonemapComponent@ GetTonemapComponent()", WRAP_MFN(Entity, GetComponent<TonemapComponent>) },
            { "BloomComponent@ GetBloomComponent()", WRAP_MFN(Entity, GetComponent<BloomComponent>) },
            { "GTAOComponent@ GetGTAOComponent()", WRAP_MFN(Entity, GetComponent<GTAOComponent>) },
            { "SSRComponent@ GetSSRComponent()", WRAP_MFN(Entity, GetComponent<SSRComponent>) },




            { "NameComponent@ RemoveNameComponent()", WRAP_MFN(Entity, RemoveComponent<NameComponent>) },
            { "TransformComponent@ RemoveTransformComponent()", WRAP_MFN(Entity, RemoveComponent<TransformComponent>) },
            { "MeshComponent@ RemoveMeshComponent()", WRAP_MFN(Entity, RemoveComponent<MeshComponent>) },
            { "MeshRendererComponent@ RemoveMeshRendererComponent()", WRAP_MFN(Entity, RemoveComponent<MeshRendererComponent>) },
            { "LightComponent@ RemoveLightComponent()", WRAP_MFN(Entity, RemoveComponent<LightComponent>) },
            { "ScriptComponent@ RemoveScriptComponent()", WRAP_MFN(Entity, RemoveComponent<ScriptComponent>) },
            { "CameraComponent@ RemoveCameraComponent()", WRAP_MFN(Entity, RemoveComponent<CameraComponent>) },
            { "RigidBodyComponent@ RemoveRigidBodyComponent()", WRAP_MFN(Entity, RemoveComponent<RigidBodyComponent>) },

            { "ProceduralSkyComponent@ RemoveProceduralSkyComponent()", WRAP_MFN(Entity, RemoveComponent<ProceduralSkyComponent>) },
            { "HDRISkyComponent@ RemoveHDRISkyComponent()", WRAP_MFN(Entity, RemoveComponent<HDRISkyComponent>) },

            { "TonemapComponent@ RemoveTonemapComponent()", WRAP_MFN(Entity, RemoveComponent<TonemapComponent>) },
            { "BloomComponent@ RemoveBloomComponent()", WRAP_MFN(Entity, RemoveComponent<BloomComponent>) },
            { "GTAOComponent@ RemoveGTAOComponent()", WRAP_MFN(Entity, RemoveComponent<GTAOComponent>) },
            { "SSRComponent@ RemoveSSRComponent()", WRAP_MFN(Entity, RemoveComponent<SSRComponent>) }
        },
        {}
    );
}

void ScriptManager::RegisterScene() const
{
    AddType("Scene", sizeof(Scene),
        {
            { "void Start()", WRAP_MFN(Scene, Start) },
            { "void Update(float deltaTime)", WRAP_MFN(Scene, Update) },
            { "Entity CreateEntity()", WRAP_MFN(Scene, CreateEntity) },
            { "void RemoveEntity(const Entity& in)", WRAP_MFN(Scene, RemoveEntity) },
            { "void ReparentEntity(Entity, Entity)", WRAP_MFN(Scene, ReparentEntity) },
            { "Entity CloneEntity(const Entity& in)", WRAP_MFN(Scene, CloneEntity) },
            { "Entity GetEntity(uint32)", WRAP_MFN_PR(Scene, GetEntity, (entt::id_type), Entity) },
            { "Entity GetEntity(const string& in)", WRAP_MFN_PR(Scene, GetEntity, (const std::string&), Entity) },
            { "bool IsChildOf(const Entity& in, const Entity& in)", WRAP_MFN(Scene, IsChildOf) },
            { "glm::mat4 GetWorldTransform(Entity)", WRAP_OBJ_LAST(as::GetWorldTransform) }
        }, {}
    );
}

}

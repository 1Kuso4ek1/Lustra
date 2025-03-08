#include <AudioManager.hpp>
#include <LLGL/Log.h>

namespace lustra
{

AudioManager::~AudioManager()
{
    ma_engine_uninit(&engine);
}

void AudioManager::Init()
{
    result = ma_engine_init(nullptr, &engine);

    if(result != MA_SUCCESS)
        LLGL::Log::Errorf(
            LLGL::Log::ColorFlags::StdError,
            "Failed to initialize audio engine\n"
        );
}

Sound AudioManager::LoadSound(const std::filesystem::path& path)
{
    Sound sound;
    result = ma_sound_init_from_file(&engine, path.string().c_str(), 0, nullptr, nullptr, sound.GetSound().get());

    if(result != MA_SUCCESS)
        LLGL::Log::Errorf(
            LLGL::Log::ColorFlags::StdError,
            "Failed to load sound %s\n",
            path.string().c_str()
        );

    return sound;
}

ma_engine* AudioManager::GetEngine()
{
    return &engine;
}

}

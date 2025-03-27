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
    if(initialized)
        return;

    result = ma_engine_init(nullptr, &engine);

    if(result != MA_SUCCESS)
        LLGL::Log::Errorf(
            LLGL::Log::ColorFlags::StdError,
            "Failed to initialize audio engine\n"
        );
    else
        initialized = true;
}

void AudioManager::RemoveSound(Sound& sound)
{
    ma_sound_uninit(sound.GetSound().get());
}

Sound AudioManager::LoadSound(const std::filesystem::path& path)
{
    if(!initialized)
    {
        LLGL::Log::Errorf(
            LLGL::Log::ColorFlags::StdError,
            "Audio engine not initialized\n"
        );

        return Sound();
    }

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

Sound AudioManager::CopySound(Sound& sound)
{
    if(!initialized)
    {
        LLGL::Log::Errorf(
            LLGL::Log::ColorFlags::StdError,
            "Audio engine not initialized\n"
        );

        return Sound();
    }

    Sound newSound;
    result = ma_sound_init_copy(&engine, sound.GetSound().get(), 0, nullptr, newSound.GetSound().get());

    if(result != MA_SUCCESS)
        LLGL::Log::Errorf(
            LLGL::Log::ColorFlags::StdError,
            "Failed to copy sound\n"
        );

    return newSound;
}

ma_engine* AudioManager::GetEngine()
{
    return &engine;
}

}

#pragma once
#include <Singleton.hpp>
#include <Sound.hpp>

#include <filesystem>

namespace lustra
{

class AudioManager : public Singleton<AudioManager>
{
public:
    ~AudioManager();

public:
    void Init();

    void RemoveSound(Sound& sound);

    Sound LoadSound(const std::filesystem::path& path);
    Sound CopySound(Sound& sound);

    ma_engine* GetEngine();

private:
    ma_engine engine;
    ma_result result;

    bool initialized = false;
};

}

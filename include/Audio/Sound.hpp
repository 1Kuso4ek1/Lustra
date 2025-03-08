#pragma once
#include <miniaudio.h>
#include <memory>

namespace lustra
{

class Sound
{
public:
    Sound();
    ~Sound();

    void Play();

    std::shared_ptr<ma_sound> GetSound();

private:
    std::shared_ptr<ma_sound> sound;
};

}

#include <Sound.hpp>

namespace lustra
{

Sound::Sound()
{
    sound = std::make_shared<ma_sound>();
}

Sound::~Sound()
{
    if(sound.unique())
        ma_sound_uninit(sound.get());
}

void Sound::Play()
{
    ma_sound_start(sound.get());
}

std::shared_ptr<ma_sound> Sound::GetSound()
{
    return sound;
}

}

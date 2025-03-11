#include <Sound.hpp>

#include <glm/gtc/quaternion.hpp>

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
    if(ma_sound_is_playing(sound.get()))
        ma_sound_seek_to_pcm_frame(sound.get(), 0);

    ma_sound_start(sound.get());
}

void Sound::Stop()
{
    ma_sound_stop(sound.get());
}

void Sound::SetPosition(const glm::vec3& position)
{
    ma_sound_set_position(sound.get(), position.x, position.y, position.z);
}

void Sound::SetVelocity(const glm::vec3& velocity)
{
    ma_sound_set_velocity(sound.get(), velocity.x, velocity.y, velocity.z);
}

void Sound::SetOrientation(const glm::quat& orientation)
{
    glm::vec3 forward = orientation * glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = orientation * glm::vec3(0.0f, 1.0f, 0.0f);

    ma_sound_set_direction(sound.get(), forward.x, forward.y, forward.z);
}

void Sound::SetDirection(const glm::vec3& direction)
{
    ma_sound_set_direction(sound.get(), direction.x, direction.y, direction.z);
}

void Sound::SetCone(const Cone& cone)
{
    ma_sound_set_cone(sound.get(), cone.inner, cone.outer, cone.outerGain);
}

void Sound::SetDirectionalAttenuationFactor(float factor)
{
    ma_sound_set_directional_attenuation_factor(sound.get(), factor);
}

void Sound::SetDopplerFactor(float factor)
{
    ma_sound_set_doppler_factor(sound.get(), factor);
}

void Sound::SetFade(float volumeBegin, float volumeEnd, uint64_t lengthMillis)
{
    ma_sound_set_fade_in_milliseconds(sound.get(), volumeBegin, volumeEnd, lengthMillis);
}

void Sound::SetRolloff(float rolloff)
{
    ma_sound_set_rolloff(sound.get(), rolloff);
}

void Sound::SetStartTime(uint64_t timeMillis)
{
    ma_sound_set_start_time_in_milliseconds(
        sound.get(),
        ma_engine_get_time_in_milliseconds(sound->engineNode.pEngine) + timeMillis
    );
}

void Sound::SetStopTime(uint64_t timeMillis)
{
    ma_sound_set_stop_time_in_milliseconds(
        sound.get(),
        ma_engine_get_time_in_milliseconds(sound->engineNode.pEngine) + timeMillis
    );
}

void Sound::SetMaxDistance(float distance)
{
    ma_sound_set_max_distance(sound.get(), distance);
}

void Sound::SetMinDistance(float distance)
{
    ma_sound_set_min_distance(sound.get(), distance);
}

void Sound::SetMaxGain(float gain)
{
    ma_sound_set_max_gain(sound.get(), gain);
}

void Sound::SetMinGain(float gain)
{
    ma_sound_set_min_gain(sound.get(), gain);
}

void Sound::SetPan(float pan)
{
    ma_sound_set_pan(sound.get(), pan);
}

void Sound::SetPitch(float pitch)
{
    ma_sound_set_pitch(sound.get(), pitch);
}

void Sound::SetVolume(float volume)
{
    ma_sound_set_volume(sound.get(), volume);
}

void Sound::SetLooping(bool looping)
{
    ma_sound_set_looping(sound.get(), looping);
}

void Sound::SetSpatializationEnabled(bool enabled)
{
    ma_sound_set_spatialization_enabled(sound.get(), enabled);
}

glm::vec3 Sound::GetPosition()
{
    auto position = ma_sound_get_position(sound.get());

    return { position.x, position.y, position.z };
}

glm::vec3 Sound::GetVelocity()
{
    auto velocity = ma_sound_get_velocity(sound.get());

    return { velocity.x, velocity.y, velocity.z };
}

glm::vec3 Sound::GetDirection()
{
    auto direction = ma_sound_get_direction(sound.get());

    return { direction.x, direction.y, direction.z };
}

Sound::Cone Sound::GetCone()
{
    Cone cone;

    ma_sound_get_cone(sound.get(), &cone.inner, &cone.outer, &cone.outerGain);

    return cone;
}

float Sound::GetDirectionalAttenuationFactor()
{
    return ma_sound_get_directional_attenuation_factor(sound.get());
}

float Sound::GetDopplerFactor()
{
    return ma_sound_get_doppler_factor(sound.get());
}

float Sound::GetRolloff()
{
    return ma_sound_get_rolloff(sound.get());
}

float Sound::GetMaxDistance()
{
    return ma_sound_get_max_distance(sound.get());
}

float Sound::GetMinDistance()
{
    return ma_sound_get_min_distance(sound.get());
}

float Sound::GetMaxGain()
{
    return ma_sound_get_max_gain(sound.get());
}

float Sound::GetMinGain()
{
    return ma_sound_get_min_gain(sound.get());
}

float Sound::GetPan()
{
    return ma_sound_get_pan(sound.get());
}

float Sound::GetPitch()
{
    return ma_sound_get_pitch(sound.get());
}

float Sound::GetVolume()
{
    return ma_sound_get_volume(sound.get());
}

bool Sound::IsLooping()
{
    return ma_sound_is_looping(sound.get());
}

bool Sound::IsSpatializationEnabled()
{
    return ma_sound_is_spatialization_enabled(sound.get());
}

std::shared_ptr<ma_sound> Sound::GetSound()
{
    return sound;
}

}

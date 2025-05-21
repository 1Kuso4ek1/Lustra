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

void Sound::Play() const
{
    if(ma_sound_is_playing(sound.get()))
        ma_sound_seek_to_pcm_frame(sound.get(), 0);

    ma_sound_start(sound.get());
}

void Sound::Stop() const
{
    ma_sound_stop(sound.get());
}

void Sound::SetPosition(const glm::vec3& position) const
{
    ma_sound_set_position(sound.get(), position.x, position.y, position.z);
}

void Sound::SetVelocity(const glm::vec3& velocity) const
{
    ma_sound_set_velocity(sound.get(), velocity.x, velocity.y, velocity.z);
}

void Sound::SetOrientation(const glm::quat& orientation) const
{
    const auto forward = orientation * glm::vec3(0.0f, 0.0f, -1.0f);

    ma_sound_set_direction(sound.get(), forward.x, forward.y, forward.z);
}

void Sound::SetDirection(const glm::vec3& direction) const
{
    ma_sound_set_direction(sound.get(), direction.x, direction.y, direction.z);
}

void Sound::SetCone(const Cone& cone) const
{
    ma_sound_set_cone(
        sound.get(),
        glm::radians(cone.inner),
        glm::radians(cone.outer),
        cone.outerGain
    );
}

void Sound::SetDirectionalAttenuationFactor(const float factor) const
{
    ma_sound_set_directional_attenuation_factor(sound.get(), factor);
}

void Sound::SetDopplerFactor(const float factor) const
{
    ma_sound_set_doppler_factor(sound.get(), factor);
}

void Sound::SetFade(const float volumeBegin, const float volumeEnd, const uint64_t lengthMillis) const
{
    ma_sound_set_fade_in_milliseconds(sound.get(), volumeBegin, volumeEnd, lengthMillis);
}

void Sound::SetRolloff(const float rolloff) const
{
    ma_sound_set_rolloff(sound.get(), rolloff);
}

void Sound::SetStartTime(const uint64_t millis) const
{
    ma_sound_set_start_time_in_milliseconds(
        sound.get(),
        ma_engine_get_time_in_milliseconds(sound->engineNode.pEngine) + millis
    );
}

void Sound::SetStopTime(const uint64_t millis) const
{
    ma_sound_set_stop_time_in_milliseconds(
        sound.get(),
        ma_engine_get_time_in_milliseconds(sound->engineNode.pEngine) + millis
    );
}

void Sound::SetMaxDistance(const float distance) const
{
    ma_sound_set_max_distance(sound.get(), distance);
}

void Sound::SetMinDistance(const float distance) const
{
    ma_sound_set_min_distance(sound.get(), distance);
}

void Sound::SetMaxGain(const float gain) const
{
    ma_sound_set_max_gain(sound.get(), gain);
}

void Sound::SetMinGain(const float gain) const
{
    ma_sound_set_min_gain(sound.get(), gain);
}

void Sound::SetPan(const float pan) const
{
    ma_sound_set_pan(sound.get(), pan);
}

void Sound::SetPitch(const float pitch) const
{
    ma_sound_set_pitch(sound.get(), pitch);
}

void Sound::SetVolume(const float volume) const
{
    ma_sound_set_volume(sound.get(), volume);
}

void Sound::SetLooping(const bool looping) const
{
    ma_sound_set_looping(sound.get(), looping);
}

void Sound::SetSpatializationEnabled(const bool enabled) const
{
    ma_sound_set_spatialization_enabled(sound.get(), enabled);
}

glm::vec3 Sound::GetPosition() const
{
    const auto [x, y, z] = ma_sound_get_position(sound.get());

    return { x, y, z };
}

glm::vec3 Sound::GetVelocity() const
{
    const auto [x, y, z] = ma_sound_get_velocity(sound.get());

    return { x, y, z };
}

glm::vec3 Sound::GetDirection() const
{
    const auto [x, y, z] = ma_sound_get_direction(sound.get());

    return { x, y, z };
}

Sound::Cone Sound::GetCone() const
{
    Cone cone{};

    ma_sound_get_cone(sound.get(), &cone.inner, &cone.outer, &cone.outerGain);

    cone.inner = glm::degrees(cone.inner);
    cone.outer = glm::degrees(cone.outer);

    return cone;
}

float Sound::GetDirectionalAttenuationFactor() const
{
    return ma_sound_get_directional_attenuation_factor(sound.get());
}

float Sound::GetDopplerFactor() const
{
    return ma_sound_get_doppler_factor(sound.get());
}

float Sound::GetRolloff() const
{
    return ma_sound_get_rolloff(sound.get());
}

float Sound::GetMaxDistance() const
{
    return ma_sound_get_max_distance(sound.get());
}

float Sound::GetMinDistance() const
{
    return ma_sound_get_min_distance(sound.get());
}

float Sound::GetMaxGain() const
{
    return ma_sound_get_max_gain(sound.get());
}

float Sound::GetMinGain() const
{
    return ma_sound_get_min_gain(sound.get());
}

float Sound::GetPan() const
{
    return ma_sound_get_pan(sound.get());
}

float Sound::GetPitch() const
{
    return ma_sound_get_pitch(sound.get());
}

float Sound::GetVolume() const
{
    return ma_sound_get_volume(sound.get());
}

bool Sound::IsLooping() const
{
    return ma_sound_is_looping(sound.get());
}

bool Sound::IsSpatializationEnabled() const
{
    return ma_sound_is_spatialization_enabled(sound.get());
}

std::shared_ptr<ma_sound> Sound::GetSound()
{
    return sound;
}

}

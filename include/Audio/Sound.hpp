#pragma once
#include <miniaudio.h>

#include <glm/glm.hpp>

#include <memory>

namespace lustra
{

class Sound
{
public:
    struct Cone { float inner; float outer; float outerGain; };

    Sound();
    ~Sound();

    void Play() const;
    void Stop() const;

    void SetPosition(const glm::vec3& position) const;
    void SetVelocity(const glm::vec3& velocity) const;
    void SetOrientation(const glm::quat& orientation) const;
    void SetDirection(const glm::vec3& direction) const;
    void SetCone(const Cone& cone) const;

    void SetDirectionalAttenuationFactor(float factor) const;
    void SetDopplerFactor(float factor) const;
    void SetFade(float volumeBegin, float volumeEnd, uint64_t lengthMillis) const;
    void SetRolloff(float rolloff) const;

    void SetStartTime(uint64_t millis) const;
    void SetStopTime(uint64_t millis) const;

    void SetMaxDistance(float distance) const;
    void SetMinDistance(float distance) const;
    void SetMaxGain(float gain) const;
    void SetMinGain(float gain) const;

    void SetPan(float pan) const;
    void SetPitch(float pitch) const;
    void SetVolume(float volume) const;

    void SetLooping(bool looping) const;
    void SetSpatializationEnabled(bool enabled) const;

    glm::vec3 GetPosition() const;
    glm::vec3 GetVelocity() const;
    glm::vec3 GetDirection() const;

    Cone GetCone() const;

    float GetDirectionalAttenuationFactor() const;
    float GetDopplerFactor() const;
    float GetRolloff() const;

    float GetMaxDistance() const;
    float GetMinDistance() const;
    float GetMaxGain() const;
    float GetMinGain() const;

    float GetPan() const;
    float GetPitch() const;
    float GetVolume() const;

    bool IsLooping() const;
    bool IsSpatializationEnabled() const;

    std::shared_ptr<ma_sound> GetSound();

private:
    std::shared_ptr<ma_sound> sound;
};

}

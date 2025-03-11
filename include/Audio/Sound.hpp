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

    void Play();
    void Stop();

    void SetPosition(const glm::vec3& position);
    void SetVelocity(const glm::vec3& velocity);
    void SetOrientation(const glm::quat& orientation);
    void SetDirection(const glm::vec3& direction);
    void SetCone(const Cone& cone);

    void SetDirectionalAttenuationFactor(float factor);
    void SetDopplerFactor(float factor);
    void SetFade(float volumeBegin, float volumeEnd, uint64_t lengthMillis);
    void SetRolloff(float rolloff);

    void SetStartTime(uint64_t millis);
    void SetStopTime(uint64_t millis);
    
    void SetMaxDistance(float distance);
    void SetMinDistance(float distance);
    void SetMaxGain(float gain);
    void SetMinGain(float gain);

    void SetPan(float pan);
    void SetPitch(float pitch);
    void SetVolume(float volume);

    void SetLooping(bool looping);
    void SetSpatializationEnabled(bool enabled);

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

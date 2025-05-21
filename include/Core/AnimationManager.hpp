#pragma once
#include <Singleton.hpp>

namespace lustra
{

class Entity;

class AnimationManager final : public Singleton<AnimationManager>
{
public:
    void Update(Entity& entity, float deltaTime);

private:

};

}

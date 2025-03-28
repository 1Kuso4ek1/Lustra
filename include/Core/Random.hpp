#pragma once
#include <Singleton.hpp>

#include <random>

namespace lustra
{

class Random : public Singleton<Random>
{
public:
    void SetSeed(uint32_t seed)
    {
        generator.seed(seed);
    }

    float Value()
    {
        return std::uniform_real_distribution<float>(0, 1)(generator);
    }

    float Range(float min, float max)
    {
        return std::uniform_real_distribution<float>(min, max)(generator);
    }

    int Range(int min, int max)
    {
        return std::uniform_int_distribution<int>(min, max)(generator);
    }

private:
    std::mt19937 generator;
};

}

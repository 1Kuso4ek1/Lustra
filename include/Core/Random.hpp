#pragma once
#include <Singleton.hpp>

#include <random>

namespace lustra
{

class Random final : public Singleton<Random>
{
public:
    void SetSeed(const uint32_t seed)
    {
        generator.seed(seed);
    }

    float Value()
    {
        return std::uniform_real_distribution<float>(0, 1)(generator);
    }

    float Range(const float min, const float max)
    {
        return std::uniform_real_distribution(min, max)(generator);
    }

    int Range(const int min, const int max)
    {
        return std::uniform_int_distribution(min, max)(generator);
    }

private:
    friend class Singleton<Random>;

    Random() : generator(device()) {}

private:
    std::random_device device;
    std::mt19937 generator;
};

}

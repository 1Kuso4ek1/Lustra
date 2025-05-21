#pragma once
#include <chrono>
#include <string_view>

#include <LLGL/Log.h>

namespace lustra
{

class Timer
{
public:
    Timer();

    void Reset();

    float GetElapsedSeconds() const;
    float GetElapsedMilliseconds() const;

private:
    std::chrono::high_resolution_clock::time_point start;
};

class ScopedTimer
{
public:
    explicit ScopedTimer(std::string_view name);
    ~ScopedTimer();

private:
    Timer timer;

    std::string_view name;
};

namespace global
{

inline Timer appTimer;

}

}

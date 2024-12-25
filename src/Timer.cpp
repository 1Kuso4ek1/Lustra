#include <Timer.hpp>
#include <chrono>
#include <iostream>

namespace dev
{

Timer::Timer()
{
    Reset();
}

void Timer::Reset()
{
    start = std::chrono::high_resolution_clock::now();
}

float Timer::GetElapsedSeconds()
{
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::duration<float>>(end - start);

    return duration.count();
}

long Timer::GetElapsedMilliseconds()
{
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    return duration.count();
}

ScopedTimer::ScopedTimer(std::string_view name) : name(name) {}

ScopedTimer::~ScopedTimer()
{
    std::cout << name << " took " << timer.GetElapsedMilliseconds() << "ms" << std::endl;
}

}

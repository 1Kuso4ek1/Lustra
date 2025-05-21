#include <Timer.hpp>
#include <chrono>

namespace lustra
{

Timer::Timer()
{
    Reset();
}

void Timer::Reset()
{
    start = std::chrono::high_resolution_clock::now();
}

float Timer::GetElapsedSeconds() const
{
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::duration<float>>(end - start);

    return duration.count();
}

float Timer::GetElapsedMilliseconds() const
{
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    return static_cast<float>(duration.count()) / 1000.0f;
}

ScopedTimer::ScopedTimer(const std::string_view name) : name(name) {}

ScopedTimer::~ScopedTimer()
{
    LLGL::Log::Printf(
        LLGL::Log::ColorFlags::Bold | LLGL::Log::ColorFlags::Blue,
        "%s took %.3f ms\n", name.data(), timer.GetElapsedMilliseconds()
    );
}

}

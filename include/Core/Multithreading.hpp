#pragma once
#include <Singleton.hpp>

#include <future>
#include <vector>

// Jesus Christ, ANOTHER Windows compatibility bullshit.
// TODO: Resolve dependencies so the Windows.h doesn't get included in non-related files
#ifdef _WIN32
    #undef AddJob
#endif

using namespace std::chrono_literals;

namespace lustra
{

class Multithreading final : public Singleton<Multithreading>
{
public:
    using Job = std::pair<std::function<void()>, std::function<void()>>;

    void Update();

    void AddJob(const Job& job);

    size_t GetJobsNum() const;

private:
    using ManagedJob = std::pair<std::future<void>, std::function<void()>>;

    std::vector<ManagedJob> jobs;
};

}

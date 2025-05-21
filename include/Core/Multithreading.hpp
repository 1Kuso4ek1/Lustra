#pragma once
#include <Singleton.hpp>

#include <future>
#include <vector>

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

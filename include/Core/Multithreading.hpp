#pragma once
#include <Singleton.hpp>

#include <vector>
#include <future>

using namespace std::chrono_literals;

namespace dev
{

class Multithreading : public Singleton<Multithreading>
{
public:
    using Job = std::pair<std::function<void()>, std::function<void()>>;

    void Update();

    void AddJob(const Job& job);

    size_t GetJobsNum() const;

private:
    /* std::vector<std::future<void>> jobs;
    std::vector<std::function<void()>> mainThread; */
    using ManagedJob = std::pair<std::future<void>, std::function<void()>>;

    std::vector<ManagedJob> jobs;
};

}

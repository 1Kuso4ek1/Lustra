#pragma once
#include <vector>
#include <future>

using namespace std::chrono_literals;

class Multithreading
{
public:
    static Multithreading& Get();

public:
    void Update();

    void AddJob(std::future<void>&& future);
    void AddJob(std::function<void()> job);

    void AddMainThreadJob(std::function<void()> job);

    size_t GetJobsNum() const;

private:
    Multithreading() {};

private:
    std::vector<std::future<void>> jobs;
    std::vector<std::function<void()>> mainThread;
};

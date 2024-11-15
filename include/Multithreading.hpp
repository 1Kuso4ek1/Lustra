#pragma once
#include <thread>
#include <vector>
#include <future>
#include <functional>

using namespace std::chrono_literals;

class Multithreading
{
public:
    static Multithreading& GetInstance();

    void Update();
    void AddJob(std::thread::id threadID, std::future<void>&& future);
    void AddMainThreadJob(std::function<void()> job);

    size_t GetJobsNum();

private:
    Multithreading() {};

    std::vector<std::future<void>> jobs;
    std::vector<std::function<void()>> mainThread;
};

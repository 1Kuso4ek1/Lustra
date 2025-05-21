#include <Multithreading.hpp>

namespace lustra
{

void Multithreading::Update()
{
    for(size_t i = 0; i < jobs.size(); i++)
    {
        if(!jobs[i].first.valid())
        {
            if(jobs[i].second)
                jobs[i].second();

            jobs.erase(jobs.begin() + static_cast<long>(i));
        }
        else if(jobs[i].first.wait_for(0ms) == std::future_status::ready)
        {
            jobs[i].second();

            jobs.erase(jobs.begin() + static_cast<long>(i));
        }
    }
}

void Multithreading::AddJob(const Job& job)
{
    auto task = job.first ? std::async(std::launch::async, job.first) : std::future<void>();

    jobs.emplace_back(std::move(task), job.second);
}

size_t Multithreading::GetJobsNum() const
{
    return jobs.size();
}

}

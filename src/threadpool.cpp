#include "threadpool.h"

void ThreadPool::thread_loop(){
    while(true){
        Job job;
        {
            auto lock = std::unique_lock{queue_mutex};
            mutex_condition.wait(lock, [this] { return !jobs.empty() || should_terminate; });
            if(should_terminate)
                return;

            job = std::move(jobs.front());
            jobs.pop();
        }
        job();
    }
}

ThreadPool::ThreadPool(){
    auto max_threads = std::thread::hardware_concurrency();
    while(max_threads-->0)
        threads.emplace_back(&ThreadPool::thread_loop, this);
}

ThreadPool::~ThreadPool(){
    auto lock = std::unique_lock{queue_mutex};
    should_terminate = true;
    mutex_condition.notify_all();
}

void ThreadPool::queue_task(Job job){
    auto lock = std::unique_lock{queue_mutex};
    jobs.push(std::move(job));
    mutex_condition.notify_one();
}
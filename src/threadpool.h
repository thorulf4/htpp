#pragma once
#include <thread>
#include <mutex>
#include <queue>
#include <functional>
#include <condition_variable>

class ThreadPool{
    using Job = std::move_only_function<void()>;
    uint16_t max_connections;

    bool should_terminate{false};
    std::vector<std::jthread> threads;
    std::mutex queue_mutex;
    std::condition_variable mutex_condition;
    std::condition_variable queue_condition;
    std::queue<Job> jobs;

    void thread_loop();
public:
    ThreadPool(uint16_t max_connections);
    ~ThreadPool();
    void queue_task(Job job);
    void wait_for_space();
};
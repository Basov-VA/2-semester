#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <chrono>
/*
 * Требуется написать класс ThreadPool, реализующий пул потоков, которые выполняют задачи из общей очереди.
 * С помощью метода PushTask можно положить новую задачу в очередь
 * С помощью метода Terminate можно завершить работу пула потоков.
 * Если в метод Terminate передать флаг wait = true,
 *  то пул подождет, пока потоки разберут все оставшиеся задачи в очереди, и только после этого завершит работу потоков.
 * Если передать wait = false, то все невыполненные на момент вызова Terminate задачи, которые остались в очереди,
 *  никогда не будут выполнены.
 * После вызова Terminate в поток нельзя добавить новые задачи.
 * Метод IsActive позволяет узнать, работает ли пул потоков. Т.е. можно ли подать ему на выполнение новые задачи.
 * Метод GetQueueSize позволяет узнать, сколько задач на данный момент ожидают своей очереди на выполнение.
 * При создании нового объекта ThreadPool в аргументах конструктора указывается количество потоков в пуле. Эти потоки
 *  сразу создаются конструктором.
 * Задачей может являться любой callable-объект, обернутый в std::function<void()>.
 */

class ThreadPool {
public:
    ThreadPool(size_t threadCount):
    active(1),
    isTerminated(0)
    {
        for(size_t i = 0; i < threadCount; ++i)
        {
            threads.emplace_back([this](){
                while (true)
                {
                    std::unique_lock<std::mutex> lock(mtx_);
                    while (tasks.size() == 0 && active == 1)
                    {
                        cvEmpty.wait(lock);
                    }
                    if(active == 0 && tasks.size() == 0)
                    {
                        this->size0.notify_one();
                        return;
                    }
                    if(isTerminated == 1)
                    {
                        return;
                    }
                    std::function<void()> task = tasks.front();
                    tasks.pop();
                    lock.unlock();
                    task();
                }

            });
        }

    }

    void PushTask(const std::function<void()>& task) {
        std::unique_lock<std::mutex> lock(mtx_);
        if(active == 0)
        {
            throw Terminated();
        }
        tasks.push(task);
        cvEmpty.notify_one();
        lock.unlock();
    }

    void Terminate(bool wait) {
        std::unique_lock<std::mutex> lock(mtx_);
        active = 0;
        if(wait == 1)
        {
            while(tasks.size() != 0)
            {
                size0.wait(lock);
            }
        }
        else{
            isTerminated = 1;
        }
        cvEmpty.notify_all();
        lock.unlock();
        for(size_t i = 0;i < threads.size(); ++i)
        {
            threads[i].join();
        }
    }

    bool IsActive() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return active == true ? 1 : 0;
    }

    size_t QueueSize() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return tasks.size();
    }

private:

    class Terminated : public std::exception{};

    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    mutable std::mutex mtx_;

    std::condition_variable size0;
    std::condition_variable cvEmpty;
    std::atomic<bool> active;
    std::atomic<bool> isTerminated;
};

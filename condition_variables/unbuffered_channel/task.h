#pragma once

#include <mutex>
#include <condition_variable>
#include <optional>

class TimeOut : public std::exception {
    const char* what() const noexcept override {
            return "Timeout";
    }
};

template<typename T>
class UnbufferedChannel {
public:
    void Put(const T& data) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_for_put.wait(lock, [this](){return info == std::nullopt;});
        info = data;
        cv_for_get.notify_one();
        cv_b.wait(lock);
    }

    T Get(std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {
        std::unique_lock<std::mutex> lock(mtx_);
        if (!cv_for_get.wait_for(lock, timeout, [this]() { return info != std::nullopt; })) {
            throw TimeOut();
        }
        T returnable = info.value();
        info.reset();
        cv_for_put.notify_one();
        cv_b.notify_one();
        return returnable;
    }
private:
    std::condition_variable cv_for_put;
    std::condition_variable cv_for_get;
    std::condition_variable cv_b;
    std::optional<T> info;
    std::mutex mtx_;
};

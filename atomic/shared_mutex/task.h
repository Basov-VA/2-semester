#include <atomic>
#include <thread>

class SharedMutex {
public:
    SharedMutex() = default;
    void unlock_shared() {
        data.fetch_sub(1, std::memory_order_release);
    }

    void unlock() {
        data.store(0, std::memory_order_release);
    }

    void lock_shared() {
        int64_t q;
        do {
            q = data.load(std::memory_order_relaxed);
            while (q == -1 * 1) {
                std::this_thread::yield();
                q = data.load(std::memory_order_relaxed);
            }
        } while (data.compare_exchange_weak(q, q + 1, std::memory_order_acquire, std::memory_order_relaxed) == 0);
    }

    void lock() {
        int64_t ex = 0;
        while (!data.compare_exchange_weak(ex, -1, std::memory_order_acquire, std::memory_order_relaxed)) {
            ex = 0;
            std::this_thread::yield();
        }
    }

private:
    std::atomic<int64_t > data;
};

#pragma once

#include <atomic>
#include <cstdint>
#include <chrono>
#include <mutex>
#include <set>
#include <algorithm>
#include <optional>
#include <iostream>
#include <stdexcept>


bool isprime(uint64_t number)
{
    if(number <= 1)
    {
        return false;
    }
    else
    {
        for(uint64_t i = 2; i * i <= number; ++i)
        {
            if(number % i == 0)
            {
                return false;
            }
        }
        return true;
    }
}

/*
 * Класс PrimeNumbersSet -- множество простых чисел в каком-то диапазоне
 */
class PrimeNumbersSet {
public:
    PrimeNumbersSet() = default;

    // Проверка, что данное число присутствует в множестве простых чисел
    bool IsPrime(uint64_t number) const {
        // for (uint64_t i = 2; i * i < number; i++) {
        //     if (number % i == 0)
        //         return 0;
        // }
        // return 1;
        return primes_.find(number) != primes_.end();
    }

    // Получить следующее по величине простое число из множества
    uint64_t GetNextPrime(uint64_t number) const {
        std::lock_guard<std::mutex> lock(set_mutex_);
        auto it = primes_.upper_bound(number);
        if (it != primes_.end()) {
            return *it;
        }
        throw std::invalid_argument("No next prime after limit");
    }


    /*
     * Найти простые числа в диапазоне [from, to) и добавить в множество
     * Во время работы этой функции нужно вести учет времени, затраченного на
     * ожидание лока мюьтекса, а также времени, проведенного в секции кода под
     * локом
     */
    void AddPrimesInRange(uint64_t from, uint64_t to) {
        std::vector<uint64_t> primenumbers;
        for (uint64_t q = from; q < to; ++q) {
            if (isprime(q)) {
                primenumbers.push_back(q);
            }
        }

        auto start = std::chrono::steady_clock::now();
        {
            std::lock_guard<std::mutex> lock(set_mutex_);
            auto end = std::chrono::steady_clock::now();
            nanoseconds_waiting_mutex_ += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

            if (!primenumbers.empty()) {
                for (auto val : primenumbers) {
                    primes_.insert(val);
                }
            }

            auto end2 = std::chrono::steady_clock::now();
            nanoseconds_under_mutex_ += std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - end).count() - nanoseconds_waiting_mutex_;
        }  // Here, unlock is automatically called for set_mutex_
    }



    // Посчитать количество простых чисел в диапазоне [from, to)
    size_t GetPrimesCountInRange(uint64_t from, uint64_t to) const {
        uint64_t count = 0;
        for(uint64_t q = from; q < to; ++q)
        {
            if(isprime(q))
            {
                ++count;
            }
        }
        return count;
    }

    // Получить наибольшее простое число из множества
    uint64_t GetMaxPrimeNumber() const {
        auto it = primes_.end();
        it--;
        return *it;
    }

    // Получить суммарное время, проведенное в ожидании лока мьютекса во время
    // работы функции AddPrimesInRange
    std::chrono::nanoseconds GetTotalTimeWaitingForMutex() const{
        return std::chrono::nanoseconds(nanoseconds_waiting_mutex_);
    }

    // Получить суммарное время, проведенное в коде под локом во время работы
    // функции AddPrimesInRange
    std::chrono::nanoseconds GetTotalTimeUnderMutex() const{
        return std::chrono::nanoseconds(nanoseconds_under_mutex_);
    }

private:
    std::set<uint64_t> primes_;
    mutable std::mutex set_mutex_;
    std::atomic<uint64_t> nanoseconds_under_mutex_=0, nanoseconds_waiting_mutex_=0;
};
#pragma once

#include <mutex>
#include <unordered_set>
#include <thread>

template<typename... Ts>
class ThreadSafeSet: public std::unordered_set<Ts...>
{
public:
    template<typename... Args>
    friend std::unique_lock<std::mutex> AcquireLock(ThreadSafeSet& container, Args... args)
    {
        return std::unique_lock<std::mutex>(container.m_m, args...);
    }

    template<typename... Args>
    friend std::unique_lock<std::mutex> AcquireLock(const ThreadSafeSet& container, Args... args)
    {
        return std::unique_lock<std::mutex>(container.m_m, args...);
    }

private:
    mutable std::mutex m_m;
};
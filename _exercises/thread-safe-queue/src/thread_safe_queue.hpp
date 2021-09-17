#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class ThreadSafeQueue
{
    std::queue<T> q_;
    mutable std::mutex mtx_q_;
    std::condition_variable cv_q_not_empty_;

public:
    bool empty() const
    {
        std::lock_guard<std::mutex> lk {mtx_q_};
        return q_.empty();
    }

    void push(const T& item)
    {
        {
            std::lock_guard<std::mutex> lk {mtx_q_};
            q_.push(item);
        }
        cv_q_not_empty_.notify_one();
    }

    void push(T&& item)
    {
        {
            std::lock_guard<std::mutex> lk {mtx_q_};
            q_.push(std::move(item));
        }
        cv_q_not_empty_.notify_one();
    }

    void push(std::initializer_list<T> items)
    {
        { 
            std::lock_guard<std::mutex> lk{mtx_q_}; // critical section starts
            for(const auto& item : items)
                q_.push(item);
        } // release a lock - critical section ends
        cv_q_not_empty_.notify_all();
    }

    bool try_pop(T& item)
    {
        std::unique_lock<std::mutex> lk{mtx_q_, std::try_to_lock};

        if (!lk.owns_lock() || q_.empty())
            return false;

        item = std::move(q_.front();
        q_.pop();
        return true;
    }

    void pop(T& item)
    {
        std::unique_lock<std::mutex> lk{mtx_q_};
        cv_q_not_empty_.wait(lk, [this] { return !q_.empty(); }); // sleeps if q_ is empty

        item = std::move(q_.front());
        q_.pop();
    }
};

#endif // THREAD_SAFE_QUEUE_HPP

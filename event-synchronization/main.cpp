#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <numeric>
#include <random>
#include <string>
#include <thread>
#include <vector>

using namespace std::literals;

namespace BusyWait
{
    class Data
    {
        std::vector<int> data_;
        std::atomic<bool> is_ready_ = false;

    public:
        void read()
        {
            std::cout << "Start reading..." << std::endl;
            data_.resize(100);

            std::random_device rnd;
            std::generate(begin(data_), end(data_), [&rnd]
                { return rnd() % 1000; });
            std::this_thread::sleep_for(2s);
            std::cout << "End reading..." << std::endl;

            is_ready_ = true; // is_ready_.store(true, std::memory_order_seq_cst)
            is_ready_.store(true, std::memory_order_release); // RELEASE
        }

        void process(int id)
        {
            /////////////////////////////////////
            //while(!is_ready) -> while(!is_ready_.load(std::memory_order_seq_cst))
            while (!is_ready_.load(std::memory_order_acquire)) // busy wait - ACQUIRE
            {
            }

            long sum = std::accumulate(begin(data_), end(data_), 0L);

            std::cout << "Id: " << id << "; Sum: " << sum << std::endl;
        }
    };
}

class Data
{
    std::vector<int> data_;
    bool is_ready_ = false;
    std::mutex mtx_is_ready_;
    std::condition_variable cv_data_ready_;

public:
    void read()
    {
        std::cout << "Start reading..." << std::endl;
        data_.resize(100);

        std::random_device rnd;
        std::generate(begin(data_), end(data_), [&rnd] { return rnd() % 1000; });
        std::this_thread::sleep_for(2s);
        std::cout << "End reading..." << std::endl;

        { 
            std::lock_guard<std::mutex> lk {mtx_is_ready_};  // ACQUIRE
            is_ready_ = true;
        } // RELEASE
        cv_data_ready_.notify_all();
    }

    void process(int id)
    {
        std::unique_lock<std::mutex> lk{mtx_is_ready_};
        // while (!is_ready_)
        // {
        //     cv_data_ready_.wait(lk);
        // }
        cv_data_ready_.wait(lk, [this] { return is_ready_; });
        lk.unlock();

        long sum = std::accumulate(begin(data_), end(data_), 0L);

        std::cout << "Id: " << id << "; Sum: " << sum << std::endl;
    }
};

int main()
{
    Data data;

    std::thread thd_producer {[&data]
        { data.read(); }};
    std::thread thd_consumer1 {[&data]
        { data.process(1); }};
    std::thread thd_consumer2 {[&data]
        { data.process(2); }};

    thd_producer.join();
    thd_consumer1.join();
    thd_consumer2.join();
}

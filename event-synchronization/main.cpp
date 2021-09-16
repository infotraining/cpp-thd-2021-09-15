#include <atomic>
#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
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

            is_ready_ = true; 
        }

        void process(int id)
        {
            while (!is_ready_)
            {
            }

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

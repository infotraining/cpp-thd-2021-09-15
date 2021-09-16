#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace std::literals;

void background_work(size_t id, const std::string& text, std::chrono::milliseconds delay)
{
    std::cout << "bw#" << id << " has started..." << std::endl;

    for (const auto& c : text)
    {
        std::cout << "bw#" << id << ": " << c << std::endl;

        std::this_thread::sleep_for(delay);
    }

    std::cout << "bw#" << id << " is finished..." << std::endl;
}

class BackgroundWork
{
    const int id_;
    const std::string text_;

public:
    BackgroundWork(int id, std::string text)
        : id_ {id}
        , text_ {std::move(text)}
    {
    }

    void operator()(std::chrono::milliseconds delay) const
    {
        std::cout << "BW#" << id_ << " has started..." << std::endl;

        for (const auto& c : text_)
        {
            std::cout << "BW#" << id_ << ": " << c << std::endl;

            std::this_thread::sleep_for(delay);
        }

        std::cout << "BW#" << id_ << " is finished..." << std::endl;
    }
};

void std_containers()
{
    {
        std::vector<int> vec = {1, 2, 3};

        std::thread thd1 {[&]
            { std::cout << vec[0] << std::endl; }};
        std::thread thd2 {[&]
            { std::cout << vec[0] << std::endl; }};
        thd1.join();
        thd2.join();
    }

    {
        std::map<int, std::string> dict = {{1, "one"}, {2, "two"}, {3, "three"}};

        std::thread thd1 {[&]
            { std::cout << dict[0] << std::endl; }}; // data race
        std::thread thd2 {[&]
            { std::cout << dict.at(0) << std::endl; }}; // OK
        thd1.join();
        thd2.join();
    }
}

void may_throw()
{
    throw 13;
}

struct Counter
{
    long counter = 0;
    std::mutex mtx_counter;

    void increment()
    {
        std::lock_guard<std::mutex> lk_mtx_counter{mtx_counter};
        ++counter;       
    } // desctor lock_guard calls unlock() on mutex
};

void increment(Counter& value)
{
    for (long i = 0; i < 1'000'000; ++i)
    {
        value.increment();
    }
}

int main()
{
    std::cout << "Main thread starts..." << std::endl;
    const std::string text = "Hello Threads";

    Counter counter;

    std::thread thd1 {increment, std::ref(counter)};
    std::thread thd2 {[&]
        { increment(counter); }};

    thd1.join();
    thd2.join();

    std::cout << "Counter: " << counter.counter << std::endl;

    std::cout << "Main thread ends..." << std::endl;
}

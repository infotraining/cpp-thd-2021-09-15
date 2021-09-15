#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "joining_thread.hpp"

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
        : id_{id}
        , text_{std::move(text)}
    {
    }

    void operator()(std::chrono::milliseconds delay) const
    {
        std::string header = "BW#";

        std::cout << header << id_ << " has started..." << std::endl;

        for (const auto& c : text_)
        {
            std::cout << "BW#" << id_ << ": " << c << std::endl;

            std::this_thread::sleep_for(delay);
        }

        std::cout << "BW#" << id_ << " is finished..." << std::endl;
    }
};

class Printer
{
public:
    void print(const std::string& msg)
    {
        for(const auto& c : msg)
        {
            std::cout << "Printing: " << c << std::endl;
            std::this_thread::sleep_for(100ms);
        }            
    }
};

int main()
{
    std::cout << "Main thread starts..." << std::endl;
    const std::string text = "Hello Threads";

    std::thread empty_thread;
    std::cout << "empty_thread id: " << empty_thread.get_id() << std::endl;

    std::thread thd1{&background_work, 1, std::cref(text), 1s};
    std::thread thd2{&background_work, 2, "THREAD2", 150ms};
    const BackgroundWork bw{3, "BackgroundWorker"};
    std::thread thd3{std::cref(bw), 100ms};
    Printer printer;

    std::thread thd4{[&printer](){ printer.print("PRINT"); }};

    const std::vector<int> source = {1, 2, 3, 4, 5, 6, 7};


    std::vector<int> target;
    std::vector<int> backup;
    
    {
        ext::joining_thread thd_copy{[&]{ std::copy(begin(source), end(source), back_inserter(target)); }};
        std::thread thd_backup{[&]{ std::copy(begin(source), end(source), back_inserter(backup)); }};
        
        thd_backup.join();
    }

    std::cout << "target: ";
    for (const auto& item : target)
        std::cout << item << " ";
    std::cout << "\n";

    std::cout << "backup: ";
    for (const auto& item : backup)
        std::cout << item << " ";
    std::cout << "\n";

    thd1.detach();
    std::cout << "Main thread is working" << std::endl;
    thd2.join();
    thd3.join();
    thd4.join();

    std::cout << "Main thread ends..." << std::endl;
}

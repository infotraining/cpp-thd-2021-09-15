#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <random>
#include <future>

using namespace std::literals;

int calculate_square(int x)
{
    std::cout << "Starting calculation for " << x << " in " << std::this_thread::get_id() << std::endl;

    std::random_device rd;
    std::uniform_int_distribution<> distr(100, 5000);

    std::this_thread::sleep_for(std::chrono::milliseconds(distr(rd)));

    if (x % 3 == 0)
        throw std::runtime_error("Error#3");

    return x * x;
}

void save_to_file(const std::string& filename)
{
    std::cout << "Saving to file: " << filename << std::endl;

    std::this_thread::sleep_for(3s);

    std::cout << "File saved: " << filename << std::endl;
}

void wtf()
{
    auto f1 = std::async(std::launch::async, &save_to_file, "Content1");
    auto f2 = std::async(std::launch::async, &save_to_file, "Content2");
    auto f3 = std::async(std::launch::async, &save_to_file, "Content3");
    auto f4 = std::async(std::launch::async, &save_to_file, "Content4");
}

int main()
{
    std::future<int> f1 = std::async(std::launch::async, &calculate_square, 9);
    std::future<int> f2 = std::async(std::launch::async, &calculate_square, 8);
    std::future<void> f3 = std::async(std::launch::async, &save_to_file, "Some content");

    while (f2.wait_for(250ms) != std::future_status::ready)
    {
        std::cout << "I'm still waiting for f2" << std::endl;
    }

    try
    {
        int result1 = f1.get();
        std::cout << "14*14 = " << result1 << "\n";
    }
    catch(const std::runtime_error& excpt)
    {
        std::cout << "Caught an exception: " << excpt.what() << "\n";
    }
    
    std::cout << "8*8 = " << f2.get() << std::endl;

    f3.wait();

    std::cout << "File is saved..." << std::endl;

    wtf();
}

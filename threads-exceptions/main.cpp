#include <cassert>
#include <chrono>
#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std::literals;

template <typename T>
struct ThreadResult
{
    T value;
    std::exception_ptr eptr;
};

void background_work(size_t id, const std::string& text, std::chrono::milliseconds delay, ThreadResult<int>& result)
{
    std::cout << "bw#" << id << " has started..." << std::endl;

    for (const auto& c : text)
    {
        std::cout << "bw#" << id << ": " << c << std::endl;

        std::this_thread::sleep_for(delay);
    }

    try
    {
        result.value = text.at(3);
    }
    catch (...)
    {
        std::cout << "something has been caught!" << std::endl;
        result.eptr = std::current_exception(); // storing caught exception in exception_ptr
        return;
    }

    std::cout << "bw#" << id << " is finished..." << std::endl;
}

int main()
{
    std::cout << "Main thread starts..." << std::endl;
    const std::string text = "Hello Threads";

    std::vector<ThreadResult<int>> results(2);
    std::vector<std::thread> thds(2);

    thds[0] = std::thread {&background_work, 1, "Text", 200ms, std::ref(results[0])};
    thds[1] = std::thread {&background_work, 2, "OK", 500ms, std::ref(results[1])};

    for (auto& thd : thds)
        thd.join();

    for (auto& r : results)
    {
        if (!r.eptr)
        {
            std::cout << "Result: " << r.value << std::endl;
        }
        else
        {
            try
            {
                std::rethrow_exception(r.eptr);
            }
            catch (const std::out_of_range& excpt)
            {
                std::cout << excpt.what() << " has been caught!" << std::endl;
            }
            catch (const std::exception& excpt)
            {
                std::cout << "An exception has been caught!" << std::endl;
            }
        }
    }

    std::cout << "Main thread ends..." << std::endl;
}

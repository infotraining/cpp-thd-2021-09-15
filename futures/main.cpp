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

void using_async()
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

void foo(int&)
{
}

void foo(int&&)
{
}

template <typename T>
void bar(T&&)
{}

void using_foo()
{
    int x = 10;
    foo(x); // foo(int&)
    foo(10); // foo(int&&)

    bar(x); // bar(int&)
    bar(10); // bar(int&&)
}

template <typename Callable>
auto launch_async(Callable&& callable)
{
    using ResultT = decltype(callable());
    
    std::packaged_task<ResultT()> pt{std::forward<Callable>(callable)};
    std::future<ResultT> f = pt.get_future();

    std::thread thd{std::move(pt)};
    thd.detach();

    return f;
}

void no_wtf()
{
    launch_async([] { save_to_file("Content1"); });
    launch_async([] { save_to_file("Content2"); });
    launch_async([] { save_to_file("Content3"); });
    auto f = launch_async([] { save_to_file("Content4"); });
    f.wait();
}

class SquareCalculator
{
    std::promise<int> promise_;
public:
    void calculate(int x)
    {
        try
        {
            int result = calculate_square(x);
            promise_.set_value(result);
        }
        catch(...)
        {
            promise_.set_exception(std::current_exception());
        }
    }

    std::future<int> get_future()
    {
        return promise_.get_future();
    }
};

void using_promise()
{
    SquareCalculator calc;

    std::future<int> f = calc.get_future();
    std::shared_future<int> shared_f = f.share();

    std::thread consumer_thd{[shared_f] {
        std::cout << "Consuming: " << shared_f.get() << std::endl;
    }};

    std::thread thd{[&] { calc.calculate(11); }};

    std::cout << "11*11 = " << shared_f.get() << std::endl;

    thd.join();
    consumer_thd.join();
}

int main()
{
    std::packaged_task<int()> pt1{[] { return calculate_square(16); }};
    std::packaged_task<int(int)> pt2{&calculate_square};
    
    std::future<int> f1 = pt1.get_future();
    std::future<int> f2 = pt2.get_future();

    std::thread thd1{std::move(pt1)};
    std::thread thd2{std::move(pt2), 7};

    std::cout << "16*16 = " << f1.get() << std::endl;
    std::cout << "7*7 = " << f2.get() << std::endl;

    thd1.join();
    thd2.join();

    no_wtf();

    using_promise();
}

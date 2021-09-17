#include "thread_safe_queue.hpp"
#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <future>
#include <random>

using namespace std::literals;

void background_work(size_t id, const std::string& text, std::chrono::milliseconds delay)
{
    std::cout << "bw#" << id << " has started in the thread#" << std::this_thread::get_id() << std::endl;

    for (const auto& c : text)
    {
        std::cout << "bw#" << id << ": " << c << " - thread#" << std::this_thread::get_id() << std::endl;

        std::this_thread::sleep_for(delay);
    }

    std::cout << "bw#" << id << " is finished..." << std::endl;
}

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

using Task = std::function<void()>;

//using FollyTask = folly::Function<void()>;

static Task end_of_work {};

namespace ver_1_0
{
    class ThreadPool
    {
        std::vector<std::thread> threads_;
        ThreadSafeQueue<Task> q_tasks_;

        void run()
        {
            while (true)
            {
                Task task;
                q_tasks_.pop(task); // waits for task

                if (sepuku_task(task))
                    return;

                task(); // executes popped task
            }
        }

        bool sepuku_task(Task& t)
        {
            return t == nullptr;
        }

    public:
        ThreadPool(size_t size)
            : threads_(size)
        {
            for (auto& thd : threads_)
            {
                thd = std::thread {[this]
                    { run(); }};
            }
        }

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        ~ThreadPool()
        {
            // send poisoning pills
            for (size_t i = 0; i < threads_.size(); ++i)
                q_tasks_.push(end_of_work);

            for (auto& thd : threads_)
                thd.join();
        }

        void submit(Task task)
        {
            q_tasks_.push(task);
        }
    };
}

namespace ver_1_1
{
    class ThreadPool
    {
        std::vector<std::thread> threads_;
        ThreadSafeQueue<Task> q_tasks_;
        bool is_done_ = false;

        void run()
        {
            while (true)
            {
                Task task;
                q_tasks_.pop(task); // waits for task

                task(); // executes popped task
                
                if (is_done_)
                    return;
            }
        }

    public:
        ThreadPool(size_t size)
            : threads_(size)
        {
            for (auto& thd : threads_)
            {
                thd = std::thread {[this]
                    { run(); }};
            }
        }

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        ~ThreadPool()
        {
            for(size_t i = 0; i < threads_.size(); ++i)
                submit([this] { is_done_ = true; });

            for (auto& thd : threads_)
                thd.join();
        }

        template <typename Callable>
        auto submit(Callable&& task)
        {
            using ResultT = decltype(task());

            auto pt = std::make_shared<std::packaged_task<ResultT()>>(std::forward<Callable>(task));
            auto fresult = pt->get_future();
            q_tasks_.push([pt] { (*pt)(); });

            return fresult;
        }
    };
}

int main()
{
    using namespace ver_1_1;

    std::cout << "Main thread starts..." << std::endl;
    const std::string text = "Hello Threads";

    std::thread thd1 {[&]
        { background_work(1, text, 10ms); }};
    thd1.join();

    ThreadPool thread_pool(4);

    thread_pool.submit([&]
        { background_work(1, text, 10ms); });

    for (int i = 0; i < 100; ++i)
        thread_pool.submit([i, &text]
            { background_work(i, text, 5ms); });

    std::vector<std::tuple<int, std::future<int>>> f_squares;

    for(int i = 1; i < 10; ++i)
    {
        f_squares.emplace_back(i, thread_pool.submit([i] { return calculate_square(i); }));
    }

    for(auto& fs : f_squares)
    {
        auto x = std::get<0>(fs);

        try
        {
            auto sq = std::get<1>(fs).get();
            std::cout << x << "*" << x << " = " << sq << std::endl;
        }
        catch(const std::exception& e)
        {
            std::cout << x << "*" << x << " = " << e.what() << std::endl;
        }        
    }
}

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <numeric>
#include <random>
#include <thread>

#include <memory>

using namespace std;

void calc_hits(long num_of_iteration, long& hits)
{
    std::mt19937_64 gen {std::random_device {}()};
    std::uniform_real_distribution<double> rnd_gen(-1.0, 1.0);

    long local_hits = 0;
    double x;
    double y;
    for (long n = 0; n < num_of_iteration; ++n)
    {
        x = rnd_gen(gen);
        y = rnd_gen(gen);
        if (x * x + y * y < 1)
            local_hits++;
    }
    hits = local_hits;
}

int main()
{
    const long N = 100'000'000;

    //////////////////////////////////////////////////////////////////////////////
    // single thread

    {
        cout << "Pi (single thread) calculation started!" << endl;
        const auto start = chrono::high_resolution_clock::now();

        long hits = 0;

        calc_hits(N, hits);

        const double pi = static_cast<double>(hits) / N * 4;

        const auto end = chrono::high_resolution_clock::now();
        const auto elapsed_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();

        cout << "Pi = " << pi << endl;
        cout << "Elapsed = " << elapsed_time << "ms" << endl;
    }
    //////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////
    // single thread

    cout << "Pi (multi threading) calculation started!" << endl;
    const auto start = chrono::high_resolution_clock::now();

    long hits = 0;

    const auto threads_number = std::max(1u, std::thread::hardware_concurrency());

    std::vector<long> hitss(threads_number);
    std::vector<std::thread> thds(threads_number);

    for(int i = 0; i < threads_number; ++i)
    {
        thds[i] = std::thread{&calc_hits, N / threads_number, std::ref(hitss[i])};
    }

    for(auto& thd : thds)
        thd.join();

    hits = std::accumulate(hitss.begin(), hitss.end(), 0L);

    const double pi = static_cast<double>(hits) / N * 4;

    const auto end = chrono::high_resolution_clock::now();
    const auto elapsed_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    cout << "Pi = " << pi << endl;
    cout << "Elapsed = " << elapsed_time << "ms" << endl;

    //////////////////////////////////////////////////////////////////////////////
}

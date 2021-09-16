#include <iostream>
#include <mutex>
#include <thread>

using namespace std;

timed_mutex mtx;

void background_worker(int id, int timeout)
{
    cout << "THD#" << id << " is waiting for mutex..." << endl;

    unique_lock lk(mtx, try_to_lock);

    if (!lk.owns_lock())
    {
        do
        {
            cout << "THD#" << id << " doesn't own lock..."
                 << " Tries to acquire mutex..." << endl;
        } while (!lk.try_lock_for(chrono::milliseconds(timeout)));
    }

    cout << "Start of THD#" << id << endl;

    this_thread::sleep_for(chrono::seconds(10));

    cout << "End of THD#" << id << endl;
}

int main()
{
    thread thd1(&background_worker, 1, 500);
    thread thd2(&background_worker, 2, 750);

    thd1.join();
    thd2.join();
}

#include <iostream>
#include <thread>
#include <mutex>

class BankAccount
{
    const int id_;
    double balance_;
    mutable std::recursive_mutex mtx_;

public:
    BankAccount(int id, double balance)
        : id_(id)
        , balance_(balance)
    {
    }

    void print() const
    {        
        std::cout << "Bank Account #" << id_ << "; Balance = " << balance() << std::endl;
    }

    void transfer(BankAccount& to, double amount)
    {
#if __cplusplus < 201703L
        std::unique_lock<std::recursive_mutex> lk_from{mtx_, std::defer_lock};
        std::unique_lock<std::recursive_mutex> lk_to{to.mtx_, std::defer_lock};
        std::lock(lk_from, lk_to); // deadlock protection

        // std::lock(mtx_, to.mtx_);
        // std::unique_lock<std::recursive_mutex> lk_from{mtx_, std::adopt_lock};
        // std::unique_lock<std::recursive_mutex> lk_to{to.mtx_, std::adopt_lock};
#else
        std::scoped_lock lks{mtx_, to.mtx_};
#endif
        balance_ -= amount;              
        to.balance_ += amount;        
    }

    void withdraw(double amount)
    {
        std::lock_guard<std::recursive_mutex> lk{mtx_};
        balance_ -= amount;
    }

    void deposit(double amount)
    {
        std::lock_guard<std::recursive_mutex> lk{mtx_};
        balance_ += amount;
    }

    int id() const
    {
        return id_;
    }

    double balance() const
    {
        std::lock_guard<std::recursive_mutex> lk{mtx_};
        return balance_;
    }

    void lock()
    {
        mtx_.lock();
    }

    void unlock()
    {
        mtx_.unlock();
    }

    bool try_lock()
    {
        return mtx_.try_lock();
    }
};

void make_withdraws(BankAccount& ba, int no_of_operations)
{
    for (int i = 0; i < no_of_operations; ++i)
        ba.withdraw(1.0);
}

void make_deposits(BankAccount& ba, int no_of_operations)
{
    for (int i = 0; i < no_of_operations; ++i)
        ba.deposit(1.0);
}

void make_transfer(BankAccount& from, BankAccount& to, int count, double amount)
{
    for(int i = 0; i < count; ++i)
        from.transfer(to, amount);
}

int main()
{
    const int NO_OF_ITERS = 1'000'000;

    BankAccount ba1(1, 1'000'000);
    BankAccount ba2(2, 1'000'000);

    std::cout << "Before threads are started: \n";
    ba1.print();
    ba2.print();

    std::thread thd1(&make_withdraws, std::ref(ba1), NO_OF_ITERS);
    std::thread thd2(&make_deposits, std::ref(ba1), NO_OF_ITERS);
    std::thread thd3{&make_transfer, std::ref(ba1), std::ref(ba2), NO_OF_ITERS, 1.0};
    std::thread thd4{&make_transfer, std::ref(ba2), std::ref(ba1), NO_OF_ITERS, 1.0};

    thd1.join();
    thd2.join();
    thd3.join();
    thd4.join();

    {
        std::lock_guard<BankAccount> lk{ba1};
        ba1.withdraw(1'000);
        ba1.deposit(2'000'000);
        ba1.withdraw(1'000'000);
        ba1.transfer(ba1, 1'000'000);
    }

    std::cout << "After all threads are done: \n";
    ba1.print();
    ba2.print();
}

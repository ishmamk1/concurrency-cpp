#include <mutex>

class some_big_object;

void swap(some_big_object& lhs, some_big_object& rhs);

class X
{
    private:
        some_big_object some_detail;
        std::mutex mutex;

    public:
        X(some_big_object const& sd) : some_detail{sd} {}

        friend void swap(X& lhs, X& rhs) {
            if (&lhs == &rhs) {
                return;
            }
            // locks both mutexes without deadlock
            std::lock(lhs.mutex, rhs.mutex);

            // adopt_lock transfers ownership and with lock_guard releasing lock is through RAII
            std::lock_guard<std::mutex> lock_a(lhs.mutex, std::adopt_lock);
            std::lock_guard<std::mutex> lock_b(rhs.mutex, std::adopt_lock);

            swap(lhs.some_detail, rhs.some_detail);
        }
};
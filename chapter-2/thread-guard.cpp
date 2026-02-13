/*
Example of Listing 2.3 Using RAII to wait for a thread to complete
*/

#include <iostream>
#include <thread>

class thread_guard
{
    std::thread& t;

    public:
        explicit thread_guard(std::thread& t_) : t(t_) {};
        ~thread_guard() {
            // Check if joinable since it can join once
            if (t.joinable()) {
                t.join();
            }
        }

        // Cannot copy thread_guard with delete keyword since two guards managing one thread is dangerous
        thread_guard(thread_guard const&)=delete;
        thread_guard& operator=(thread_guard const&)=delete;
};

struct func {
    int& i;

    func(int& i_) : i{i_} {};

    void operator()() {
        for (int j = 0; j < 1000000; j++) {
            std::cout << j + 1 << std::endl;
        }
    }
};

int main() {
    int local_state = 0;
    func myfunc(local_state);
    std::thread t(myfunc);

    /* If you uncomment line 48 and comment 49, you'll get the original case 
    where the function will end before the thread can start.

    If you keep only line 49, then it will work as it should.
    */

    //t.detach();
    thread_guard guard(t);
}


#include <exception>
#include <memory>
#include <mutex>
#include <stack>

#include <iostream>
#include <thread>
#include <chrono>

struct empty_stack : std::exception
{
    const char* what() const noexcept override {
        return "empty stack";
    }
};

template<typename T>
class threadsafe_stack
{
    private:
        std::stack<T> stack;
        mutable std::mutex mutex;
    public:
        threadsafe_stack() {}
        threadsafe_stack(const threadsafe_stack& other) {
            std::lock_guard<std::mutex> lock(other.mutex);
            stack = other.stack;
        }

        // Copy assignment operator is deleted
        threadsafe_stack& operator=(const threadsafe_stack&) = delete;

        void push(T new_value) {
            std::lock_guard<std::mutex> lock(mutex);
            stack.push(new_value);
        }

        std::shared_ptr<T> pop() {
            std::lock_guard<std::mutex> lock(mutex);
            if (stack.empty()) throw empty_stack();

            std::shared_ptr<T> const res(std::make_shared<T>(stack.top()));
            stack.pop();
            return res;
        }

        void pop(T& value) {
            std::lock_guard<std::mutex> lock(mutex);
            if (stack.empty()) throw empty_stack();
            value = stack.top();
            stack.pop();
        }

        bool empty() {
            std::lock_guard<std::mutex> lock(mutex);
            return stack.empty();
        }
};

// Custom function to pop and cout a value from a threadsafe_stack
void pop_function(threadsafe_stack<int>& stack, int thread_id) {
    try {
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::shared_ptr<int> value = stack.pop();
        std::cout << thread_id << "  " << *value << "\n" ;
    } catch (const empty_stack& e) {
        std::cout << "Empty stack" << '\n';
    }
}

// main function to test the threadsafe stack for an instance where two threads attempt to pop a value
int main() {
    threadsafe_stack<int> stack;
    stack.push(8);

    std::thread t1(pop_function, std::ref(stack), 1);
    std::thread t2(pop_function, std::ref(stack), 2);

    t1.join();
    t2.join();
    return 0;
}
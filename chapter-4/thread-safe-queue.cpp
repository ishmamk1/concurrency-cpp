#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class threadsafe_queue {
    private:
        mutable std::mutex mutex; // Mutable type since copy constructor param is const
        std::queue<T> queue;
        std::condition_variable cv;
    public:
        threadsafe_queue() {};

        threadsafe_queue(const threadsafe_queue& other) {
            std::lock_guard<std::mutex> lock(other.mutex);
            queue = other.queue;
        }

        void push(T& value) {
            std::lock_guard<std::mutex> lock(mutex);
            queue.push(value);
            cv.notify_one()
        };

        void wait_and_pop(T& value) {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [this]{ return !queue.empty()} );
            value = queue.front();
            queue.pop();
        }

        std::shared_ptr<T> wait_and_pop() {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [this]{ return !queue.empty( )});
            std::shared_ptr<T> result(std::make_shared<T>(queue.front()));
            queue.pop();
            return result;
        }

        bool try_pop(T& value) {
            std::lock_guard<std::mutex> lock(mutex);
            if (queue.empty()) {
                return false;
            }
            value = queue.front();
            queue.pop();
            return value;
        }

        std::shared_ptr<T> try_pop() {
            std::lock_guard<std::mutex> lock(mutex);
            if (queue.empty()) {
                return false;
            }
            std::shared_ptr<T> result(std::make_shared<T>(queue.front()));
            queue.pop();
            return result;
        }

        bool empty() const {
            std::lock_guard<std::mutex> lock(mutex);
            return queue.empty();
        }
};

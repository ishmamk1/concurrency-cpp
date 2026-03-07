#include <exception>
#include <memory>
#include <mutex>
#include <queue>

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>


// Built blocking thread-safe queue
struct empty_queue : std::exception
{
    const char* what() const noexcept override {
        return "empty queue";
    }
};

template<typename T>
class threadsafe_queue {
    public:
        threadsafe_queue(){};
        threadsafe_queue(const threadsafe_queue<T>& other) {
            std::unique_lock<std::mutex> lock(other.mutex);
            queue = other.queue;
        }

        // Copy assignment operator is deleted
        threadsafe_queue& operator=(const threadsafe_queue&) = delete;

        void push(T value) {
            std::unique_lock<std::mutex> lock(mutex);
            queue.push(std::move(value));

            // Unlock before notifying as it can be done without being locked
            lock.unlock();

            // Notifies atleast one thread that there is a new item
            cv.notify_one();
        }

        std::shared_ptr<T> pop() {
            std::lock_guard<std::mutex> lock(mutex);
            while (queue.empty()) throw empty_queue();

            std::shared_ptr<T> result = std::make_shared<T>(queue.front());
            queue.pop();
            return result;
        };

        void pop(T& value) {
            std::lock_guard<std::mutex> lock(mutex);
            if (queue.empty()) throw empty_queue();

            value = queue.front();
            queue.pop();
        };

        bool empty() const {
            std::lock_guard<std::mutex> lock(mutex);
            return queue.empty();
        }

        std::shared_ptr<T> wait_and_pop() {
            std::unique_lock<std::mutex> lock(mutex);
            while (queue.empty()) cv.wait(lock);

            std::shared_ptr<T> result = std::make_shared<T>(queue.front());
            queue.pop();
            return result;
        };

        void wait_and_pop(T& value) {
            std::unique_lock<std::mutex> lock(mutex);
            while (queue.empty()) cv.wait(lock);

            value = queue.front();
            queue.pop();
        };

    private:
        std::queue<T> queue;
        mutable std::mutex mutex; 
        std::condition_variable cv;   
};

// Producer + Consumer Test with wait_and_pop()

const int NUM_PRODUCERS = 4;
const int NUM_CONSUMERS = 4;
const int ITEMS_PER_PRODUCER = 500;

std::atomic<int> consumed_count{ 0 };

int main() {
    threadsafe_queue<int> q;

    int total_items = NUM_PRODUCERS * ITEMS_PER_PRODUCER;

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    // Producer threads
    for (int p = 1; p <= NUM_PRODUCERS; p++) {
        producers.emplace_back([&q, p]() {
            for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
                q.push(p * ITEMS_PER_PRODUCER + i);
            }
        });
    }

    // Consumers digest any amount of items as they can <= total_count
    for (int c = 0; c < NUM_CONSUMERS; c++) {
        consumers.emplace_back([&q, total_items]() {
            while (true) {
                int current = consumed_count.load();

                if (current >= total_items) break;

                auto item = q.wait_and_pop();
                consumed_count.fetch_add(1);

                // std::cout << "Consumed: " << *item << std::endl;
            }
        });
    }

    for (auto& t : producers) t.join();

    // Suspends mainthread so producer and consumer threads can run until all items consumed
    while (consumed_count.load() < total_items) {
        std::this_thread::yield();
    }

    for (auto& t : consumers)
        t.join();

    std::cout << "Produced: " << total_items << std::endl;
    std::cout << "Consumed: " << consumed_count.load() << std::endl;

    if (consumed_count.load() == total_items)
        std::cout << "Test PASSED" << std::endl;
    else
        std::cout << "Test FAILED" << std::endl;

    return 0;


};


// Single Thread Test
/*
int main() {
    threadsafe_queue<int> q;

    q.push(1);
    q.push(2);
    q.push(3);

    std::cout << *q.pop() << std::endl;
    std::cout << *q.pop() << std::endl;

    int val;
    q.pop(val);
    std::cout << val << std::endl;

    std::cout << "Empty: " << q.empty() << std::endl;

    try {
        q.pop();
    } catch (const empty_queue& e) {
        std::cout << e.what() << std::endl;
    }
}
*/



// Producer Test (multiple threads pushing)
/*
threadsafe_queue<int> q;

void producer(int id) {
    for (int i = 0; i < 50; i++) {
        q.push(id*100+ i);
    }
}

int main() {
    std::thread t1(producer, 1);
    std::thread t2(producer, 2);
    std::thread t3(producer, 3);

    t1.join();
    t2.join();
    t3.join();

    
    while(!q.empty()) {
        std::cout << *q.pop() << std::endl;
    }

    int val;
    while(!q.empty()) {
        q.pop(val);
        std::cout << val << std::endl;
    }
}
*/



// Producer + Consumer Test

/*
threadsafe_queue<int> q;

void producer(int id) {
    for (int i = 0; i <= 50; i++) {
        q.push(id*100+i);
        std::cout << "Produced"
    }
}

void consumer() {
    for (int i = 0; i <= 50; i++) {
        try {
            auto val = q.pop();
            std::cout << "Consumed: " << *val << std::endl;
        } catch (...) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            i--; // retry
        }
    }
}

int main() {
    std::thread p(producer, 1);
    std::thread c(consumer);

    p.join();
    c.join();

}
*/


// Stress Test

/*
threadsafe_queue<int> q;

void producer(int id) {
    for (int i = 0; i < 50; i++) {
        q.push(id*100+i);
    }
}

int main(){
    std::thread t1(producer, 1);
    std::thread t2(producer, 2);
    std::thread t3(producer, 3);
    std::thread t4(producer, 4);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    int count = 0;

    while (!q.empty()) {
        q.pop();
        count++;
    }
    std::cout << count << std::endl;
};
*/


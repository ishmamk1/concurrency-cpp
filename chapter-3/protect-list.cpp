// Listing 3.1 Protecting a list with a mutex

#include <list>
#include <mutex>
#include <algorithm>

// for testing the list functions
#include <thread>
#include <vector>
#include <iostream>

std::list<int> some_list;
std::mutex some_mutex;

//cout mutex

std::mutex cout_mutex;

void add_to_list(int new_value) {
    std::lock_guard<std::mutex> guard(some_mutex);

    some_list.push_back(new_value);
}

bool list_contains(int value_to_find) {
    std::lock_guard<std::mutex> guard(some_mutex);
    return std::find(some_list.begin(), some_list.end(), value_to_find) != some_list.end();
}

int main() {
    std::vector<std::thread> threads;

    // Spawn 10 threads adding numbers
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([i]() {
            for (int j = 0; j < 3; j++) {
                int value = i * 1000 + j;
                add_to_list(value);
                
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "Thread " << i << " added " << value << "\n";
            }
        }
        );
    }

    for (auto& t : threads) {
        t.join();
    }
    std::cout << "Done!\n";
}



#include <iostream>
#include <thread>

// Listing 2.7 
std::vector<int> vector;
void print(int i) {
    vector.push_back(i);
}

int main() {
    std::vector<std::thread> threads;

    for (int i = 1; i <= 10; i++) {
        threads.push_back(std::thread(print, i));
    }
    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

    for (const auto val : vector) {
        std::cout << val << std::endl;
    }
}
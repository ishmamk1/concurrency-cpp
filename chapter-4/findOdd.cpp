// Practice to understand promise + futures
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <future>

using namespace std;
using namespace std::chrono;
typedef long int ull;

void findOdd(std::promise<ull>&& OddSumPromise, ull start, ull end) {
    ull OddSum = 0;
    for (int i = start; i <= end; ++i) {
        if (i & 1) OddSum += 1;
    }
    OddSumPromise.set_value(OddSum);
}

int main() {
    ull start = 0, end = 1900000000;

    std::promise<ull> OddSumPromise;
    std::future<ull> OddFuture = OddSumPromise.get_future();

    cout << "Thread created!\n";
    std::thread t1(findOdd, std::move(OddSumPromise), start, end);

    cout << "Waiting for result!\n"; // If we assume this work takes 5 seconds and the thread takes 3, it will only take 5 seconds to complete in total.

    cout << "Odd sum: " << OddFuture.get() << endl;
    
    t1.join();
    return 0;
}
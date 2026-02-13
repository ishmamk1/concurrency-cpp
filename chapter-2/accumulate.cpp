#include <iostream>
#include <thread>
#include <numeric>

// Does an accumulation on a block
template<typename Iterator, typename T>
struct accumulate_block {
    void operator() (Iterator first, Iterator last, T& result) {
        result = std::accumulate(first, last, result);
    }
};

template<typename Iterator,typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    unsigned long const length=std::distance(first,last);

    if (!length) {
        return init;
    }

    // Config from textbook
    unsigned long const min_per_thread=25;
    unsigned long const max_threads = (length+min_per_thread-1)/min_per_thread; 

    unsigned long const hardware_threads= std::thread::hardware_concurrency();

    unsigned long const num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads;  

    // Stores computed sums from each thread
    std::vector<T> results(num_threads);
    std::vector<std::thread> threads(num_threads-1);

    Iterator block_start=first;

    // created a thread that accumulates a block and places it in index i
    for(unsigned long i=0;i<(num_threads-1);++i)
    {
        Iterator block_end=block_start;
        std::advance(block_end,block_size);
        threads[i]=std::thread(
        accumulate_block<Iterator,T>(),
        block_start,block_end,std::ref(results[i]));
        block_start=block_end;
    }
    // accumulate any remaining vals
    accumulate_block<Iterator,T>()(block_start,last,results[num_threads-1]);

    // join threads if not completed
    std::for_each(threads.begin(),threads.end(), std::mem_fn(&std::thread::join));

    // sums all the items in result
    return std::accumulate(results.begin(),results.end(),init);
}

int main() {
    std::vector<int> vec;
    for (int i = 0; i <= 100; i++) {
        vec.push_back(i);
    }

    std::cout << parallel_accumulate(vec.begin(), vec.end(), 0) << std::endl;
}
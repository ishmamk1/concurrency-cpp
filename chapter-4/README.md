# Notes

# Chapter 4

Conditional variables and futures help syncronizing operations between threads.

std::future – a handle to get a value or exception from a task once it’s ready; use it when you need to wait for a result from another thread.

std::async – runs a function asynchronously and returns a future automatically; use it to easily run tasks in the background without manually creating threads.

std::packaged_task – wraps a callable into a task whose result can be retrieved via a future; use it when you want to store a task and run it later in another thread or thread pool.

std::promise – allows one thread to set a value or exception for another thread to receive via a future; use it when you need to explicitly send data or errors between threads.

## 4.1 Waiting for an event or other condition

If one thread is waiting for a second thread to finish a task it can
- Check a flag in shared data, but the thread consumes valuable processing time checking the flag and when the mutex is locked by the waiting thread, it cant be locked by another thread.

- have the waiting thread sleep for small periods using `std::this_thread::sleep_for()`

```cpp
bool flag;
std::mutex m;

void wait_for_flag()
{
    std::unique_lock<std::mutex> lk(m);
    while(!flag)
    {
        lk.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        lk.lock();
    }
}
```

This is an improvement but the sleep time matters and its hard to get the sleep period right.

- Use facilities from the STL such as the conditional variable where it is a variable that is associated with an event or condition and one or more threads can wait for that condition to be satisfied. When the condition is satisfied, it can notify one or more threads waiting on the condition variable to wake it up to continue processing.

## 4.1.1 Waiting for a condition with condition variables

STL provides `std::conditional_variable` which only works with `std::mutex`
It also provides `std::conditional_variable_any` which works with anything that is mutex-like.

`notify_one()` notifies one of the waiting threads if there is one.

You use `std::unique_lock` instead of `lock_guard` as the waiting thread must unlock the mutex while waiting and lock it again afterwards. If the mutex remained locked while the thread was sleeping, the producer thread would not be able to have its condition satified.

spurious wake - when a waiting thread reacquires a mutex from another thread but it isn't due to a notiflication from another thread.

## 4.1.2 Building a thread-safe queue with condition variables

# ALREADY BUILT THIS IN PROJECTS/ BUT THIS IS THE TEXTBOOK'S VARIATION

Conditional variables are also useful when theres more than one thread waiting for the same event. Using notify_one() notifies one of the waiting threads (cannot be controlled which one)

If there are multiple waiting threads and they are all waiting for the same event and all of them need to respond, then use notify_all().

## 4.2 Waiting for one-off events with futures

If a thread needs to wait for a specific one-off event, it can obtain a future for the event.

The thread can then periodically wait on the future for short periods of time to see if the event has occured while doing another task in between checks.

A future may have data with it or it may not.

There are two types called unique features `std::future` and shared feature `std::shared_future`, similar to unique and shared ptrs. 

Multiple instances of shared futures refer to the same event, which all these instances become ready at the same time.

`std::future<void>` or `std::shared_future<void>` should be used when there is no data attached to it.

You still need syncronization (Mutex, etc) if multiple threads want to access a single future, but you can use shared_future to resolve that and achieve the same result.

## 4.2.1 Returning values from background tasks

Lets say you want to run a new thread to compute a background task, the issue is threads don't provide a direct method to transferring the result of the task back. 

`std::async` is used to start an asyncronous task for which you don't need to result right away.

`std::async` holds a `std::future object` which holds the return value of the function.


Listing 4.6: Using std::future to get the return value of an asynchronous task  
```cpp
#include <future>
#include <iostream>
int find_the_answer_to_ltuae();
void do_other_stuff();
int main()
{
    std::future<int> the_answer=std::async(find_the_answer_to_ltuae);
    do_other_stuff();
    std::cout<<"The answer is "<<the_answer.get()<<std::endl;
}
```

`std::async` allows us to pass additional arguments to the fucntion by adding extra arguments to the call in the same way `std::thread` does. 

First argument points to a member function,
Second argument provides the object on which to apply the member function,
Remaining arguments are passed as arguments of the member function

Similar to threads, if the arguments are r-values, the copies are created by moving the original values.

Listing 4.7 in `pass-arguments-async.cpp`

The parameter of `std::launch` can either be 
`std::launch::deferred` to indicate that the function call is to be deferred until wait() or get() is called on the future
`std::launch::async` to indicate that the function must run on its own thread.

## 4.2.2 Associating a task with a future

`std::packaged_task<>` ties a future to a function or callable object. When it is invoked it called the function or callable and makes the future ready with the return value in the data.

If a large operation can be divided into self-contained subtasks, each of them can be wrapped in a packaged_task<> and that instance can be passed into a task scheduler or thread pool.

The template tells C++ what the function looks like such as <`ReturnType(Arg1, Arg2, ...)`>

```cpp
int add(int x, int y) return x + y;

std::packaged_task<int(int, int)> task(add);
auto future = task.get_future();
task(3,5);
std::cout << future.get() >> std::endl;
```

## Passing tasks between threads 

Listing 4.9 Running code on a GUI thread using std::packaged_task, explained in `gui.cpp`

## 4.2.3 Making std::promise

`std::promise` provides a means for setting a value T, which can later be read through an associated `std::future<T>` object.

You can obtain the `std::future` object associated with a given promise by calling get_future() similar to packaged_task. When the value of the promise is set, the future becomes ready and can be used to retrieve the stored value.

Listing 4.10 in `connections.cpp`

## 4.2.4 Saving an exception for the future


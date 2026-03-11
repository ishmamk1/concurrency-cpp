# Notes

# Chapter 4

Conditional variables and futures help syncronizing operations between threads.

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


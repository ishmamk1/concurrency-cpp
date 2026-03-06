# Notes

## Chapter 3

One benefit of using threads for concurrency is the potential to eaasily and directly share data between them.

## 3.1

if all shared data is read-only, there is no problem as the data read by one thread can't be affected by another reading the same data.

Invariant - statements true about a particular data structure
(ex: next on a linked list will take you to the next node)

The simplest problem with modifying shared data between threads is broken invariants.
One thread reads the linked list while another disconnects a node can cause the first thread to read an incomplete list.

## 3.1.1 Race Conditions

Race condition - flaw where a systems behavior depends on the uncontrolable timing of certain events.

Data race - specific type of race condition due to a concurrent modification; leads to undefined behavior

Race Conditions typically occur when you have to modify two or more distinct pieces of data, as they must be modified in separate instructions and another thread may access or change the data during the same time.

## 3.1.2  Avoiding problematic race conditions

One solution is to wrap your data structure under a protection mechanism, so only the thread modifying the data can see the states where the invariants are broken.

Another solution is to modify the design of the data struct. and invariants so the modifications are atomic/indivisible, which is lock-free programming.

Another solution is to handle the updates to the data structure as a transaction, where the modifications are stored in a log and committed in a single step. If a step cant proceed since the DS has been modified by another thread, the transaction is restarted.

## 3.2 Protecting shared data with mutexes

Mutex - syncronization primitive used in multithreaded programs to ensure only one program as access to a shared resource at a time.

## 3.2.1 Using mutexes in C++

Example shown in `protect-list.cpp`

Ensure that your member functions don't return a pointer or reference to the protected data, since it can be modified even if the mutex is currently locked since you have direct access to the variable.

## 3.2.2 Structuring code for protecting shared data

Make sure functions dont return pointers/references of the data to other functions you cannot control.

Listing 3.2 shows how you can pass in a function that contains a reference to the protected data and obtain it outside of the mutex lock

## 3.2.3 Spotting race conditions intherent in interfaces

In the example of the doubly-linked list, protecting access of each node is still dangerous and you should resort to protecting the whole list.

In terms of a stack, in single-threaded code, using `empty()` to check if a stack is empty before you check the `top()` is fine.

However, inbetween the `empty()` and `top()` call, if a stack decides to `pop()` the last element, we run into a race condition. 

This isn't an issue with the mutex, but instead the interface of the data structure.

`empty()/top()` problem is checking that the stack isn't empty (one val left) and another thread pops it before you call top, leading to undefined behavior

Other scenario is

```
Stack = [A]

Step 1 :Thread 1 calls top() → sees A

Step 2: Thread 2 calls top() → also sees A

Step 3: Thread 1 calls pop() → removes A

Step 4: Thread 2 calls pop() → removes next item (or crashes if empty)
```

One solution is combining the calls to `top()` and `pop()` but leads to an issue if the copy constructor for the objects on the stack throw an exception.

For example imagine `stack<vector<int>>`, you need to allocate memory on the heap to copy a vector and if this allocation fails, you get a `bad-alloc` exception.

If a pop was called and you remove it from the stack but the constructor didn't copy it, you lose the value.

How do we safely remoce and return a value from a thread-safe stack without causing race conditions or exception problems?

Avoid returning data in a way that can return an exception + avoid loss of data

Option 1: Pass in a reference: 

```cpp
// pop puts the popped value in result
std::vector<int> result;
some_stack.pop(result);
```

Disadvantage is that it needs to construct an instance of the stack type which can be expensive or impossible if the type needs params.

Option 2: Require a no-throw copy contructor or move contructor

Only allow types where returnign by value can't throw but this is very restrictive adn many types don't guarantee a no-throw copy.

Option 3: Return a pointer to the popped item

Advantage is that pointers can be freely copied without throwing an exception

Disadvantage is returning a pointer requires a means of memory management and for simple types like ints, it can exceed the cost of just returning by value.

`std::shared_ptr` is useful in this situation

Option 4: Provide both option 1 + (option 2 or option 3)

Listing 3.5 contains a threadsafe_stack

Problem with mutex is that if a singular mutex controls too many aspects, it can lock two threads accessing two separate pieces of data.

## 3.2.4 Deadlock: the problem and a solution

Deadlock - when two or more processes are permanently blocked because one is waiting for a resource held by another.

One solution is to lock in the same order, so always lock mutex A before mutex B, but this can still raise a deadlock

Assume you have two mutexes used to lock components exchanging data, if you have a function `func(mutex A, mutex B)` which locks the mutex that appears in the parameter first, if you call this and between mutex A being locked another call for the opposite `func(mutex B, mutex A)` locks B before the first can, then you have a deadlock.

C++ has `std::lock` which can lock two or more mutexes at once without risk of deadlock.
It provides all or nothing locking and it can't help if locks are acquired separately, it is up to the developer to avoid deadlock.

## 3.2.5 Further guidelines for avoiding deadlock

You can cause deadlocks by calling `.join()` for other threads as they all wait for the other to complete.

Avoid nested locks - Don't acquire a lock if you hold one. Deadlocks cans till occur through other means like threads waiting on each other but it removes mutex locks

Avoid calling user-supplied code while holding a lock - if the user supplied code has a lock, then you end up having nested locks and can get a deadlock

Acquire locks in a fixed order - If you need to acquire two or more locks and they cant be simutaneously, then acquire them in the same order for each thread. 

One example is the linked list example where we hold a lock to one node and only release when we acquired the lock to the next node.

Use a lock hierarchy - divide application into layers and identify all the mutexes that can be locked in a layer. 

10000  ← high level
 5000
  100  ← low level

you can go 10000 → 5000 → 100 but not 100 → 10000 

Deadlocks can occur with any syncronization construct that can lead to a wait cycle. 

Bad idea to wait for a thread while holding a lock as that thread might need to acquire the lock to continue.

## 3.2.6 Flexible locking with `std::unique_lock`

`std::unique_lock` is a more flexible version of `std::lock_guard`, as it has the same RAII functionality but it allows you to manually lock and unlock the mutex as well.

It contains a flag to determine if it is locked/unlocked which makes it perform slighly slower than `std::lock_guard`

It also allows for the use of `defer_lock` which leaves the mutexes unlocked during construction, incase you instantiate several unique locks but want to lock them simutaneously.

## 3.2.7 Transferring mutex ownership between scopes

Mutex ownership can be changed with `std::move` and this transfer is automatic if the source is an r-value but needs to be called if it is an l-value.

`std::unique_lock` is movable but not copyable.

## 3.2.8 Locking at an appropriate granularity

Lock a mutex only while actually accesing the data and do processing of the data outside of the lock.

`std::unique_lock` works well here as you can manually unlock the lock to do the other work.

```cpp
void get_and_process_data()
{
    std::unique_lock<std::mutex> my_lock(the_mutex); 
    some_class data_to_process=get_next_data_chunk();
    my_lock.unlock(); // Unlock mutex after getting data
    result_type result=process(data_to_process);
    my_lock.lock(); // Lock mutex when writing result
    write_result(data_to_process,result);
}
```

In general, a lock should be held for only the minimum possible time needed to perform the required operations.

If you don’t hold the required locks for the entire duration of an operation, you’re exposing yourself to race conditions.

## 3.3 Alternative facilities for protecting shared data

One instance is protecting shared data during initialization and then afterwards no syncronization is required (potentially read only operations after). A mutex would be overkill in this instance.

## 3.3.1 Protecting shared data during initialization

Lets say theres an expensive resource that is expensive to construct like a database connection.

One way to intialize with a mutex is by creating a unique_lock and then checking if we need to initialize within the locked segment.

```cpp
std::shared_ptr<some_resource> resource_ptr;
std::mutex resource_mutex;
void foo()
{
 std::unique_lock<std::mutex> lk(resource_mutex);
 if(!resource_ptr)
 {
 resource_ptr.reset(new some_resource);
 }
 lk.unlock();
 resource_ptr->do_something();
}
```

Another method is double locking where outside of the unique_lock !resource_ptr is checked and within the lock it is checked again.

this can cause race conditions as the read outside the lock isnt synced with the write by another thread in the lock. 

`std::once_flag` and `std::call_once` aim to resolve this as instead of locking a mutex and checking the pointer, each thread can use the call_once, which has lower overhead than a mutex. 

There is a potential race condition with initialization over static variables as multiple threads might all try to initialize instance simutaneously, but this was fixed in C++11 and later.

## 3.3.2 Protecting rarely updated data structures

read-writer mutex, which allows exclusive access by a single writer thread and concurrent access to multiple reader threads.

There isn't one in the STL, but you can use `boost::shared_mutex`

```cpp
#include <map>
#include <string>
#include <mutex>
#include <boost/thread/shared_mutex.hpp>
class dns_entry;
class dns_cache
{
    std::map<std::string,dns_entry> entries;
    mutable boost::shared_mutex entry_mutex;

    public:
        dns_entry find_entry(std::string const& domain) const
        {
            boost::shared_lock<boost::shared_mutex> lk(entry_mutex);
            std::map<std::string,dns_entry>::const_iterator const it=
            entries.find(domain);
            return (it==entries.end())?dns_entry():it->second;
        }
        void update_or_add_entry(std::string const& domain, dns_entry const& dns_details)
        {
            std::lock_guard<boost::shared_mutex> lk(entry_mutex);
            entries[domain]=dns_details;
        }
};
```

In this example, find_entry allows for multiple reader threads since boost::shared_lock protects it. However, lock_guard is used for update_or_add_entry, to provide exclusive access while the table is updated.

## 3.3.3 Recursive Locking

`std::recursive_mutex` allows the same thread to lock the mutex multiple times where a normal `std::mutex` cannot be locked twice by the same thread.

However, it is usually bad design choice since it can lead to sloppy thinking and bad design choices as class invariants can be broken when the lock is held.

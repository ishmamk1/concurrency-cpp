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

Continue on page 68


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



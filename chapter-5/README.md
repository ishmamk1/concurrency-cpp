# Notes

# Chapter 5: The C++ Memory Model and Operations on Atomic Types

## 5.1 Memory model basics

There are two parts to the memory model:
- Structural aspect: how things are laid out in memory
- Concurrent aspect: relating to concurrency

### 5.1.1 Objects and memory locations

The C++ Standard defines an object as “a region of storage,” although it goes on to assign properties to these objects, such as their type and lifetime.

A bit field is a data structure component which allows you to specify how many bits a variable should occupy in memory.

- Every variable is an object, including members of other objects
- Every obj occupies atleast one memory location
- Variables of fundamental types (int, char) are exactly one memory location
- Adjacent bit fields are apart of the same memory location

### 5.1.2 Objects, memory locations, and concurrency

If two threads want to access the same memory locations, you would either need to use a mutex or to use the synchronization properties of atomic operations.

You will still need to enforce an ordering with the atomic operations, but one thing that helps avoid undefined behavior is by using the atomic operations to access the memory location involved (does't remove the data race.)

### 5.1.3 Modification orders

Every object has a modification order — a sequence of all writes to it across all threads, starting from initialization
All threads must agree on this order within a given execution (though it may differ between runs)

Non-atomic objects = your responsibility — you must add synchronization manually; disagreement between threads = data race = undefined behavior

Atomic objects = compiler's responsibility — it inserts the necessary synchronization automatically

No speculative execution allowed — once a thread observes a value at position N in the order, all its future reads must return values from position N onward, never earlier

Per-object agreement only — threads must agree on each object's modification order individually, but not on the relative ordering across different objects 

## 5.2 Atomic operations and types in C++

An atomic operation is an indivisible operation and is either done or not done. 

### 5.2.1 The standard atomic types

Atomic types can be found in the <atomic> header. Atomics can also use emulation, where there is a is_lock_free() member function, which determines if the operations on a type is being done directly with atomic instructions, or using a lock.

`std::atomic_flag` is the only type that does not have the is_lock_free() member function.

Standard atomic types are not copyable or assignable, but they support assignment from or convert to built-in types.
They also support the compound assignment operators where appropriate: +=, -=, *=

The return value from the assignment operators and member functions is either the value stored (in the case of the
assignment operators) or the value prior to the operation (in the case of the named
functions). 

The std::atomic<> class template isn’t just a set of specializations, though. It does have a primary template that can be used to create an atomic variant of a user-definedtype. 

Because it’s a generic class template, the operations are limited to load(), store() (and assignment from and conversion to the user-defined type), exchange(), compare_exchange_weak(), and compare_exchange_strong().

Each of the operations on the atomic types has an optional memory-ordering argument that can be used to specify the required memory-ordering semantics

### 5.2.2 Operations on std::atomic_flag

std::atomic_flag is the simplest standard atomic type, which represents a Boolean flag.
It is not mean't to be in use but used as a building block. It also must be initialized with ATOMIC_FLAG_INIT
It is the only type guaranteed to be lock-free.

You can’t copy-construct another std::atomic_flag object from the first, and you can’t assign one std::atomic_flag to another.

Memory orderings — how much synchronization you want around an atomic operation:

memory_order_relaxed — just do the atomic op, no restrictions on reordering. Threads agree on the value but nothing else Cheapest.

memory_order_acquire — used on a read/load. Nothing in the current thread can move before this load. "I'm acquiring a lock — let me see everything that happened before."

memory_order_release — used on a write/store. Nothing in the current thread can move after this store. "I'm releasing — everything I did is now visible."

memory_order_acq_rel — both acquire and release on the same operation. Used on read-modify-write ops like exchange(). Acts as a full fence in both directions.

memory_order_seq_cst — the default and strongest. All threads see all sequentially consistent operations in the same global order. Most expensive but easiest to reason about.

It is used to create a spin-lock mutex seen below. This lock does a busy-wait in lock() so it is a poor choice if you expect there to be a degree of contention, but still offers mutual exclusion.

std::atomic_flag is so limited that it can’t even be used as a general Boolean flag, because it doesn’t have a simple nonmodifying query operation. For that you’re better off using std::atomic<bool>.

```cpp
class spinlock_mutex
{
    std::atomic_flag flag;

    public:
        spinlock_mutex(): flag(ATOMIC_FLAG_INIT){}

        void lock() {
            while(flag.test_and_set(std::memory_order_acquire));
        }
        
        void unlock() {
            flag.clear(std::memory_order_release);
        }
};
```

### 5.2.3 Operations on std::atomic<bool>

On page 112



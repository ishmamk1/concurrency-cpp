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

Not copy-constructible or copy-assignable, you can construct it from a nonatomic bool, so it can be initially true or false, and you can also assign to instances of
std::atomic<bool> from a nonatomic bool:

```cpp
std::atomic<bool> b(true);
b=false;
```

Similar to other atomic types, it returns the value and not the reference. If a reference to the atomic variable was returned, then any code depending on the result of the assignment would have to load the value, potentially getting the result of a modification of another thread.

Uses store() for writes and exchange() to allow you to replae the storec value with a new one of your choosing.

This new operation is called compare/exchange, and it comes in the form of the compare_exchange_weak() and compare_exchange_strong() member functions. It is similar to compare_and_swap from OS class.

c_a_e_weak() store might not be successful even if the original value is equal to the expected value, leaving the value unchanged and the return val be false.

It can fail spurriously, reason for failure is due to timing instead of the value of the variables, so the compare & exchange weak should be used in a loop.

```cpp
bool expected=false;
extern atomic<bool> b; // set somewhere else
while(!b.compare_exchange_weak(expected,true) && !expected);
```

compare_exchange_strong() is guaranteed to return false only if the actual value wasn’t equal to the expected value, so theres no need for a loop.

The compare/exchange functions are also unusual in that they can take two memory ordering parameters. This allows for the memory-ordering semantics to differ in the
case of success and failure;

`std::atomic<bool>` may not be lock-free so use the is_lock_free() member function to check if it is.

### 5.2.4 Operations on std::atomic<T*>: pointer arithmetic

The atomic form of a pointer to some type T is std::atomic<T*>, just as the atomim form of bool is std::atomic<bool>. 

It’s neither copy-constructible nor copy-assignable, although it can be both constructed and assigned from the suitable pointer values. 

`std::atomic<T*>` also has load(), store(), exchange(), compare_exchange_weak(), and compare_exchange_strong()
member functions, with similar semantics to those of std::atomic<bool>, again taking and returning T* rather than bool.

The basic operations are provided by the fetch_add() and fetch_sub() member functions, which do atomic addition and subtraction on the stored address,
and the operators += and -=, and both pre- and post-increment and decrement with ++ and --, which provide convenient wrappers.

if x is std::atomic<Foo*> to the first entry of an array of Foo objects, then x+=3 changes it to point to the fourth entry and returns a plain
Foo* that also points to that fourth entry

fetch_add() and fetch_sub() are slightly different in that they return the original value (so x.fetch_add(3) will update x to point
to the fourth value but return a pointer to the first value in the array).


`p.fetch_add(3,std::memory_order_release);`

Because both fetch_add() and fetch_sub() are read-modify-write operations, they can have any of the memory-ordering tags and can participate in a release sequence.

### 5.2.5 Operations on standard atomic integral types

In combination to the normal operations to load(), store(), exchange(), etc: atomic integral types such as std::atomic<int> or std::atomic<unsigned long long>
have:
- fetch_add(), fetch_sub(), fetch_and(), fetch_or(), fetch_xor()
- +=, -=, &=, |=, and ^=
- ++x, x++, --x, and x--

NO DIVISION, MULTIPLICATION, OR SHIFT OPERATORS

### 5.2.6 The std::atomic<> primary class template

In order to use std::atomic<UDT> for some user-defined type UDT, this type must have a trivial copy-assignment operator.
This means that the type must not have any virtual functions or virtual base classes and must use the compiler-generated copy-assignment operator.

Finally, the type must be bitwise equality comparable. This goes alongside the assignment requirements; not only must you be able to copy an object of type UDT using memcpy(), but you must be able to compare instances for equality using memcmp().

Restrictions exist to avoid:
- calling user code while holding internal locks
- deadlocks / slow comparisons
- exposing protected data

If your UDT is the same size as (or smaller than) an int or a void*, most common platforms will be able to use atomic instructions for std::atomic<UDT>. 

For more complex data, youre better off using a mutex.

### 5.3 Synchronizing operations and enforcing ordering


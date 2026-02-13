# Notes

# 2.1

## 2.1.1 Launching a thread

`std::thread` works with any callable type (function, functor, lambda)

In this case, the function object is copied into the storage of the thread and invoked from there, so the copy should behave like the original.

---

#### Example

```cpp
class background_task
{
public:
    void operator()() const
    {
        do_something();
        do_something_else();
    }
};

background_task f;
std::thread my_thread(f);
```

---

C++ most vexing parse: when you pass a temp instead of a named variable and it is treated as a function declaration.

`
std::thread my_thread(background_task());
`

To avoid this you can use double parenthesis or initialization braces

```cpp
std::thread my_thread((background_task()));
std::thread my_thread{background_task()};
```

Lambda expressions also avoid this situation.

---

Once thread is started, you need to decide whether to wait (join) or let it run on its own (detach)

Thread can terminate before you join or detach.

If you DON'T wait (join), then make sure the thread still has access to the data. 

One situation where there can be a problem if you create a thread inside a function named A that uses a local variable and the function A terminates while thread is still using the local variable.

To handle this, you can make the thread function self contained and copy the data into the thread.

Or, make sure the thread finishes before the function exits (join)

Bad idea to create a thread in a function with access to local variables in the function.

---

## 2.1.2 Waiting for a thread to complete

Use `join()` on the `std::thread` instance; it forces the function with the thread to wait till the thread is completed to terminate

`join()` slso cleans up the storage associated wit the thread so the `std::thread` object no longer associed with the finished thread. It can only be called once and is no longer joinable after.

---

## 2.1.2 Waiting in exceptional circumstances

Ensure you called `join()` or `detach()` before a thread object is destroyed

You can call detach after the thread has started

Be careful where you place the `join` since an exception or error can be thrown before the call to `join`

One solution is to call join on exception cases as well.

```cpp
struct func;
void f()
{
    int some_local_state=0;
    func my_func(some_local_state);
    std::thread t(my_func);
    try
        {
            do_something_in_current_thread();
        }
    catch(...)
        {
            t.join();
            throw;
        }
    t.join();
}
```

---

Another way is using the standard Resource Acquisition Is Initialization (RAII) idiom and build a class that does a join in its destructor.

Ensure that it cannot be copied as two guards managing a thread is dangerous.

Example is in `thread-guard.cpp`

---

## 2.1.4 Running threads in the background

Once threads are detached, ownership is transfered to the C++ RunTime library.

---

# 2.2 Passing arguments to a thread function

One method is passing arguments into the `std::thread` constructor, but they will be COPIED even if the function uses references

```
void f(int i,std::string const& s);
std::thread t(f,3,”hello”);
```

Conversely, if you wanted a thread to modify a value by reference, since it copies it, the changes get discarded and the original isn't affected.

```cpp
void update_data_for_widget(widget_id w, widget_data& data);

void oops_again(widget_id w)
{
    widget_data data;
    std::thread t(update_data_for_widget,w,data);
    display_status();
    t.join();
    process_widget_data(data);
}
```

To fix this, you need to wrap your argument with `std::ref` and change the thread to:

`std::thread t(update_data_for_widget,w,std::ref(data));`

---

Some types can only be moved instead of copied, like `std::unique_ptr` since it represents exclusive ownership.

Such dynamic objects need `std::move` to enter a thread

```cpp
void process_big_object(std::unique_ptr<big_object>);
std::unique_ptr<big_object> p(new big_object);
p->prepare_data(42);

std::thread t(process_big_object,std::move(p));
```

The ownership of the big_object is transfered into the storage of the thread, than into the `process_big_object function`

`std::thread` sharea the same situation where it can only be moved and not copied.

---

# 2.3 Transferring ownership of a thread

When you move a thread into another thread that is executing, it doesn't drop what it currently has.

# 2.4 Choosing the number of threads at runtime

`std::thread::hardware_concurrency()` returns an indication of the number of threads that can truly run concurrently in a given execution of a program.

Example in accumulate.cpp

---

# 2.5 Identifying threads

`get_id` can be used to identify a thread, and this id object can be copied and compared. 

Type `std::thread::id` is hashable and comparable.





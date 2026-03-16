// Listing 4.7 Passing arguments to a function with std::async

#include <string>
#include <future>

struct X
{
    void foo(int, std::string const& );
    std::string bar(std::string const& );
};

X x;
auto f1 = std::async(&X::foo, &x, 42, "hello"); // Calls x->foo(42, "hello") where x is a reference
auto f2=std::async(&X::bar,x,"goodbye"); // calls tmpx->bar("goodbye") where tmpx is a copy of x

struct Y
{
 double operator()(double);
};

Y y;
auto f3=std::async(Y(),3.141); // Creates copy of Y
auto f4=std::async(std::ref(y),2.718); // Reference of Y

X baz(X&); 
std::async(baz,std::ref(x)); // Calls baz(x)

class move_only
{
    public:
        move_only();
        move_only(move_only&&)
        move_only(move_only const&) = delete;
        move_only& operator=(move_only&&);
        move_only& operator=(move_only const&) = delete;
        void operator()();
};

auto f5=std::async(move_only()); // Calls tmp() where tmp is constructed from std::move(move_only())

auto f6=std::async(std::launch::async,Y(),1.2); // Runs in new thread
auto f7=std::async(std::launch::deferred,baz,std::ref(x)); // Runs in wait() or get()
auto f8=std::async(
 std::launch::deferred | std::launch::async,
 baz,std::ref(x)); // Implementation chooses

auto f9=std::async(baz,std::ref(x)); 
f7.wait(); // Invokes the deferred function
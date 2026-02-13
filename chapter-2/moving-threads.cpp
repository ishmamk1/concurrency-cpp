#include <iostream>
#include <thread>

void hello() {
    std::cout << "Hello world!" << std::endl;
}

void hello2() {
    std::cout << "Hello world again!" << std::endl;
}

int main() {
    std::thread t1(hello);
    std::thread t2=std::move(t1);
    t1=std::thread(hello2);
    std::thread t3;
    t3=std::move(t2);
    t1=std::move(t3); 
}
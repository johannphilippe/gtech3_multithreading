
#include <iostream>
#include<thread>

thread_local int i = 0;

void set_local(int n)
{
    i = n;
}

void print()
{
    std::cout << i << std::endl;
}

void thread_function(int id)
{
    set_local(id);
    ++i; 
    print();
}

int main()
{
    std::thread t1(thread_function, 1);
    std::thread t2(thread_function, 2);
    std::thread t3(thread_function, 3);
    std::thread t4(thread_function, 4);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    return 0;
}


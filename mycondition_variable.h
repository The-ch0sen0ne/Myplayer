#ifndef MYCONDITION_VARIABLE_H
#define MYCONDITION_VARIABLE_H
#include<mutex>
#include<condition_variable>

class MyCondition_Variable
{
public:
    MyCondition_Variable();

    void lock();
    void unlock();
    void signal();
    void broadcast();
    void wait();
private:
    std::mutex mutex_;
    std::condition_variable con_;
};

#endif // MYCONDITION_VARIABLE_H

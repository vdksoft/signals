/*===================================================================
*   Copyright (c) Vadim Karkhin. All rights reserved.
*   Use, modification, and distribution is subject to license terms.
*   You are welcome to contact the author at: vdksoft@gmail.com
===================================================================*/

#include <thread>
#include <atomic>

#include "demo.h"

using std::string;
using vdk::context;
using vdk::exec;
using vdk::signals_execute;

namespace
{
void function(int arg1, int arg2)
{
    std::cout << "function(" << arg1 << ", "
        << arg2 << ") from thread:"
        << std::this_thread::get_id() << std::endl;
}

struct functor
{
    explicit functor(int data) noexcept
        : data_{ data }
    {}
    void operator()(int arg1, int arg2)
    {
        std::cout << "functor(" << arg1 << ", "
            << arg2 << ") from thread:"
            << std::this_thread::get_id() << std::endl;
    }
    bool operator==(const functor & other) const noexcept
    {
        return data_ == other.data_;
    }
    int data_;
};

class demo_class : public context
{
public:

    demo_class() = default;

    void method(int arg1, int arg2)
    {
        std::cout << "demo_class::method(" << arg1 << ", "
            << arg2 << ") from thread:"
            << std::this_thread::get_id() << std::endl;
    }
};

} // namespace

void signals_cross_thread_calls()
{
    // An object of a class derived from 'context' has thread affinity
    // i.e., the object lives in the thread that created it.
    // In order to receive cross-thread signal emissions that thread
    // must call signals_execute() in a loop.

    vdk::signal<void(int, int)> sig;

    // Flag is used here for simple start synchronization
    std::atomic_bool flag = false;

    std::thread th{ [&sig, &flag]
    {
        // Note! 'object' is created in this thread, not in the main
        demo_class object;
        sig.connect(&object, &demo_class::method);

        // Also, 'object' can be used to direct cross thread signal
        // emissions for functions, function objects and lambdas
        sig.connect(&object, function);
        sig.connect(&object, functor{ 4 });
        sig.connect(&object, [](int arg1, int arg2)
        {
            std::cout << "I am lambda(" << arg1 << ", "
                << arg2 << ") from thread:"
                << std::this_thread::get_id() << std::endl;
        });
        flag = true;

        // This loop is very important! It extracts and executes all
        // slot calls received in the current thread, no matter what
        // thread signal emission originated from. Hence, slots may
        // not be thread-safe themselves; every call from any thread
        // will be serialized in this thread anyway.
        // Also, signals_execute() calls can be incorporated into any
        // existing event loop, such as window messaging system.
        int number_of_calls = 0;
        while (number_of_calls < 4)
            if (signals_execute()) ++number_of_calls;

        // Note! As soon as the thread exits and destroys the 'context'
        // object all slots associated with the object will not be
        // invoked anymore.
    } };

    // Give another thread a chance to start
    while (!flag.load()) std::this_thread::yield();

    std::cout << "signal emission from thread:" <<
        std::this_thread::get_id() << std::endl;

    sig.emit(5, 10);

    if (th.joinable()) th.join();

    std::cout << "-------------------------------" << std::endl;

    // By default, signal automatically detects what thread 'context'
    // object belongs to. There are two options:
    // 1) if a signal is emitted from the same thread the object
    // lives in, its slots will be called synchronously.
    // 2) if a signal is emitted from a different thread than the
    // object lives in, all its slot calls will be transferred into
    // the appropriate thread.
    // This default behavior can be changed by explicitly specifying
    // how each connected slot should be executed.
    demo_class object;
    // This functor will always be called asynchronously
    sig.connect(&object, functor{ 1 }, exec::async);
    // This function will always be called synchronously
    sig.connect(&object, function, exec::sync);

    // Let's try to emit the signal
    std::cout << "signal emission from thread:" <<
        std::this_thread::get_id() << std::endl;

    sig.emit(100, 200);

    // One of the two slots (exec::sync) is called synchronously during
    // the emission. Another call (exec::async) has been transferred to
    // the thread associated with the 'context' object, which happens
    // to be the current thread. So we need to get it out of there.
    // Therefore:
    signals_execute();
}
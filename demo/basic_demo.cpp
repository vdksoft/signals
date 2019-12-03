/*===================================================================
*   Copyright (c) Vadim Karkhin. All rights reserved.
*   Use, modification, and distribution is subject to license terms.
*   You are welcome to contact the author at: vdksoft@gmail.com
===================================================================*/

#include "demo.h"

// All operations shown in this demo apply to both: 'normal'
// (thread-safe) and 'lite' (single-threaded) versions of signals.
// Therefore, vdk::lite::signal can be used instead of vdk::signal.

namespace
{
void function(int arg)
{
    std::cout << "function(" << arg << ")" << std::endl;
}

struct functor
{
    explicit functor(int data) noexcept
        : data_{ data }
    {}
    void operator()(int arg)
    {
        std::cout << "functor(" << arg << ")" << std::endl;
    }
    bool operator==(const functor & other) const noexcept
    {
        return data_ == other.data_;
    }
    int data_;
};

class demo_class
{
public:

    demo_class() = default;

    void method(int arg)
    {
        std::cout << "demo_class::method(" << arg << ")" << std::endl;
    }
};

} // namespace

void signals_basic_demo()
{
    // Create a signal with no connected slots
    vdk::signal<void(int)> sig;

    // Connect the signal to a function
    sig.connect(function);
    // Connect the signal to a function object
    sig.connect(functor{ 5 });

    // Create an object to call methods on
    demo_class object;
    // Connect the signal to a method
    sig.connect(&object, &demo_class::method);

    auto lambda = [](int arg)
    {
        std::cout << "I am lambda(" << arg << ")" << std::endl;
    };

    // Connect the signal to the lambda
    // 'connect()' method returns connection 'id' to disconnect it later
    auto id = sig.connect(lambda);

    // Emit the signal
    sig.emit(5);
    // Emit the signal using another syntax
    sig(10);

    std::cout << "-------------------------------" << std::endl;

    // Disconnect the function
    // 'disconnect()' returns 'true' if the given slot has been
    // disconnected successfully, 'false' otherwise
    if (sig.disconnect(function))
        std::cout << "function has been disconnected" << std::endl;

    // Disconnect the function object
    if (sig.disconnect(functor{ 5 }))
        std::cout << "functor has been disconnected" << std::endl;

    // Disconnect the method
    if (sig.disconnect(&object, &demo_class::method))
        std::cout << "method has been disconnected" << std::endl;

    // Lambdas are not equality comparable, so we have to use ids
    // returned from 'connect()' method to disconnect them
    // Note! If a callable target provides accessible equality
    // comparison operator it is much more convenient to disconnect
    // that target using the syntax shown above than using ids
    if (sig.disconnect(id))
        std::cout << "lambda has been disconnected" << std::endl;

    std::cout << "-------------------------------" << std::endl;

    // signal connects and disconnects equal slots one by one
    sig.connect(function);
    sig.connect(function);
    sig.connect(function);

    // Now the signal has 3 identical connections to 'function'
    sig.emit(15);
    // Disconnect one of them
    sig.disconnect(function);
    // Now only 2 connections to 'function' remain in the signal
    sig(20);

    // We can disconnect all remaining connections at once
    sig.disconnect();
    // Now the signal has no connected slots again
    sig.emit(100); // nothing happens

    std::cout << "-------------------------------" << std::endl;

    sig.connect(function);
    // signals can be temporarily blocked
    // 'block()' returns whether the signal was blocked before the call
    if (!sig.block())
        std::cout << "signal was not blocked" << std::endl;
    // We can check whether the signal is blocked now
    if (sig.blocked())
        std::cout << "signal is blocked" << std::endl;
    sig.emit(110); // nothing happens; the signal is blocked
    
    // Unblock the signal
    if (sig.block(false))
        std::cout << "signal was blocked" << std::endl;
    // Again, we can check whether the signal is blocked now
    if (!sig.blocked())
        std::cout << "signal is not blocked" << std::endl;
    sig.emit(110); // the signal is working again
}
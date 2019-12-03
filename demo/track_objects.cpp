/*===================================================================
*   Copyright (c) Vadim Karkhin. All rights reserved.
*   Use, modification, and distribution is subject to license terms.
*   You are welcome to contact the author at: vdksoft@gmail.com
===================================================================*/

#include "demo.h"

// All operations shown in this demo apply to both: 'normal'
// (thread-safe) and 'lite' (single-threaded) versions of signals.
// Therefore, vdk::lite::signal can be used instead of vdk::signal and
// vdk::lite::context instead of vdk::context.

using std::string;
using vdk::context;

namespace
{
void function(const string & arg)
{
    std::cout << "function(" << arg << ")" << std::endl;
}

struct functor
{
    explicit functor(int data) noexcept
        : data_{ data }
    {}
    void operator()(const string & arg)
    {
        std::cout << "functor(" << arg << ")" << std::endl;
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

    void method(const string & arg)
    {
        std::cout << "demo_class::method(" << arg << ")" << std::endl;
    }
};

} // namespace

// Instances of classes derived from 'context' are always tracked.
// Moreover, they can be used to track not only their own method-slots,
// but also any other callable targets such as function object, lambda,
// or function (except, of cause, methods of other classes).
// Any callable target can be associated with such a 'context' object,
// just as if it were a method of the object's class. As soon as the
// object gets destroyed, its methods and all other callable targets
// associated with it are no longer reachable for signal emissions.

void signals_track_objects()
{
    vdk::signal<void(const string &)> sig;

    {
        // This object provides a context for slot invocations, so it
        // is always tracked
        demo_class object;

        // Connect its method as a slot
        sig.connect(&object, &demo_class::method);

        // Connect other callable targets as slots
        // Now they all are associated with the 'context' object
        sig.connect(&object, function);
        sig.connect(&object, functor{ 4 });
        sig.connect(&object, [](const string & arg)
        {
            std::cout << "I am lambda(" << arg << ")" << std::endl;
        });
        sig.emit("text"); // object is alive, all slots are called
    }

    // Nothing happens, because the 'context' object has been destroyed;
    // its method as well as other associated callable targets are no
    // longer reachable for the signal emission.
    sig.emit("nothing");

    // Note!
    // When you disconnect slots associated with some 'context' object
    // you should provide that object to signal's 'disconnect()' only
    // if the slot being disconnected is a method. For example:
    demo_class object;
    // Method requires an object
    sig.connect(&object, &demo_class::method);
    sig.disconnect(&object, &demo_class::method); // object is given

    // All other callable targets are standalone entities and do not
    // need the associated object in signal's 'disconnect()' method
    sig.connect(&object, function);
    sig.disconnect(function); // no associated object is given
    sig.connect(&object, functor{ 5 });
    sig.disconnect(functor{ 5 }); // no associated object is given

    sig.emit("nothing again!"); // all slots have been disconnected
}

#include <gmock/gmock.h>
#include <memory>
#include <signals.h>

using namespace ::testing;

namespace
{
void free_function(int& arg)
{
    ++arg;
}

struct struct_static
{
    static void static_slot(int& arg) { ++arg; }
};

struct struct_functor
{
    void operator()(int& arg) { ++arg; }
};

struct test_struct
{
    int foo = 0;
    test_struct(){};
    test_struct(int arg)
        : foo{arg}
    {
    }
};

bool operator==(const test_struct& lhs, const test_struct& rhs)
{
    return lhs.foo == rhs.foo;
}

class mock
{
public:
    MOCK_METHOD0(slot_no_arg, void());
    MOCK_METHOD1(slot_int, void(int));
    MOCK_METHOD1(slot_int_ptr, void(int*));
    MOCK_METHOD1(slot_const_int_ptr, void(const int*));
    MOCK_METHOD1(slot_int_ref, void(int&));
    MOCK_METHOD1(slot_const_int_ref, void(const int&));

    MOCK_METHOD1(slot_struct, void(test_struct));
    MOCK_METHOD1(slot_struct_ptr, void(test_struct*));
    MOCK_METHOD1(slot_const_struct_ptr, void(const test_struct*));
    MOCK_METHOD1(slot_struct_ref, void(test_struct&));
    MOCK_METHOD1(slot_const_struct_ref, void(const test_struct&));

    MOCK_METHOD2(slot_int_struct, void(int, test_struct));
};
} // namespace

TEST(signals, constructor)
{
    // clang-format off
    { vdk::signal<void()> s; (void)s; }
    { vdk::signal<void(int)> s{}; (void)s; }
    { vdk::signal<void(test_struct)> s{0}; (void)s; }
    { vdk::signal<void(test_struct*)> s{1}; (void)s; }
    { vdk::signal<void(const test_struct&)> s{3}; (void)s; }
    { vdk::signal<void(const test_struct&, int*)> s{6}; (void)s; }
    { vdk::signal<void(decltype(free_function))> s; (void)s; }
    // clang-format on
    SUCCEED();
}

TEST(signals, free_function)
{
    vdk::signal<void(int&)> s;
    EXPECT_TRUE(s.connect(&free_function));
    EXPECT_TRUE(s.connected(&free_function));

    int count = 0;
    s.emit(count);
    EXPECT_EQ(1, count);
}

TEST(signals, static_member_function)
{
    vdk::signal<void(int&)> s;
    EXPECT_TRUE(s.connect(&struct_static::static_slot));
    EXPECT_TRUE(s.connected(&struct_static::static_slot));

    int count = 0;
    s.emit(count);
    EXPECT_EQ(1, count);
}

TEST(signals, member_function)
{
    vdk::signal<void()> s;
    mock m;
    EXPECT_TRUE(s.connect(&m, &mock::slot_no_arg));
    EXPECT_TRUE(s.connected(&m, &mock::slot_no_arg));
    EXPECT_CALL(m, slot_no_arg()).Times(1);
    s.emit();
}

TEST(signals, member_function_on_tracked_object)
{
    vdk::signal<void()> s;
    {
        auto m = std::make_shared<mock>();
        EXPECT_TRUE(s.connect(m, &mock::slot_no_arg));
        EXPECT_TRUE(s.connected(m, &mock::slot_no_arg));
        EXPECT_CALL(*m, slot_no_arg()).Times(1);
        s.emit();
    }

    // Clean-up happens on emit, so need to emit first before checking the disconnection happened.
    s.emit();
    EXPECT_TRUE(s.empty());
}

TEST(signals, functor)
{
    vdk::signal<void(int&)> s;
    struct_functor test_s;
    EXPECT_TRUE(s.connect(&test_s));
    EXPECT_TRUE(s.connected(&test_s));

    int count = 0;
    s.emit(count);
    EXPECT_EQ(1, count);
}

TEST(signals, tracked_functor)
{
    vdk::signal<void(int&)> s;
    int count = 0;
    {
        auto test_s = std::make_shared<struct_functor>();
        EXPECT_TRUE(s.connect(test_s));
        EXPECT_TRUE(s.connected(test_s));

        s.emit(count);
        EXPECT_EQ(1, count);
    }

    // Clean-up happens on emit, so need to emit first before checking the disconnection happened.
    s.emit(count);
    EXPECT_EQ(1, count);
    EXPECT_TRUE(s.empty());
}

TEST(signals, lambda)
{
    vdk::signal<void()> s;
    int count = 0;
    auto lambda = [&]() { ++count; };
    EXPECT_TRUE(s.connect(&lambda));
    EXPECT_TRUE(s.connected(&lambda));
    s.emit();
    EXPECT_EQ(1, count);
}

TEST(signals, signal)
{
    vdk::signal<void()> s1;
    vdk::signal<void()> s2;
    EXPECT_TRUE(s1.connect(&s2));
    EXPECT_TRUE(s1.connected(&s2));
    mock m;
    s2.connect(&m, &mock::slot_no_arg);
    EXPECT_CALL(m, slot_no_arg()).Times(1);
    s1.emit();
}

TEST(signals, arg_combination)
{
    mock m;
    {
        vdk::signal<void()> s;
        s.connect(&m, &mock::slot_no_arg);
        EXPECT_CALL(m, slot_no_arg()).Times(1);
        s.emit();
    }
    {
        vdk::signal<void(int)> s;
        s.connect(&m, &mock::slot_int);
        int arg = 4;
        EXPECT_CALL(m, slot_int(arg)).Times(1);
        s.emit(arg);
    }
    {
        vdk::signal<void(int*)> s;
        s.connect(&m, &mock::slot_int_ptr);
        int arg = 4;
        EXPECT_CALL(m, slot_int_ptr(&arg)).Times(1);
        s.emit(&arg);
    }
    {
        vdk::signal<void(const int*)> s;
        s.connect(&m, &mock::slot_const_int_ptr);
        int arg = 4;
        EXPECT_CALL(m, slot_const_int_ptr(&arg)).Times(1);
        s.emit(&arg);
    }
    {
        vdk::signal<void(int&)> s;
        s.connect(&m, &mock::slot_int_ref);
        int arg = 4;
        EXPECT_CALL(m, slot_int_ref(Ref(arg))).Times(1);
        s.emit(arg);
    }
    {
        vdk::signal<void(const int&)> s;
        s.connect(&m, &mock::slot_const_int_ref);
        int arg = 4;
        EXPECT_CALL(m, slot_const_int_ref(Ref(arg))).Times(1);
        s.emit(arg);
    }
    {
        vdk::signal<void(test_struct)> s;
        s.connect(&m, &mock::slot_struct);
        test_struct arg{4};
        EXPECT_CALL(m, slot_struct(arg)).Times(1);
        s.emit(arg);
    }
    {
        vdk::signal<void(test_struct*)> s;
        s.connect(&m, &mock::slot_struct_ptr);
        test_struct arg;
        EXPECT_CALL(m, slot_struct_ptr(&arg)).Times(1);
        s.emit(&arg);
    }
    {
        vdk::signal<void(const test_struct*)> s;
        s.connect(&m, &mock::slot_const_struct_ptr);
        test_struct arg;
        EXPECT_CALL(m, slot_const_struct_ptr(&arg)).Times(1);
        s.emit(&arg);
    }
    {
        vdk::signal<void(test_struct&)> s;
        s.connect(&m, &mock::slot_struct_ref);
        test_struct arg;
        EXPECT_CALL(m, slot_struct_ref(Ref(arg))).Times(1);
        s.emit(arg);
    }
    {
        vdk::signal<void(const test_struct&)> s;
        s.connect(&m, &mock::slot_const_struct_ref);
        test_struct arg;
        EXPECT_CALL(m, slot_const_struct_ref(Ref(arg))).Times(1);
        s.emit(arg);
    }
    {
        vdk::signal<void(int, test_struct)> s;
        s.connect(&m, &mock::slot_int_struct);
        int arg1 = 4;
        test_struct arg2;
        EXPECT_CALL(m, slot_int_struct(arg1, arg2)).Times(1);
        s.emit(arg1, arg2);
    }
}

TEST(signals, repeated_connection)
{
    vdk::signal<void(int&)> s;
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(size_t(0), s.size());
    EXPECT_TRUE(s.connect(&free_function));
    EXPECT_FALSE(s.connect(&free_function)); // repeated connection
    EXPECT_TRUE(s.connected(&free_function));
    EXPECT_FALSE(s.empty());
    EXPECT_EQ(size_t(1), s.size());
}

TEST(signals, multiple_connection)
{
    vdk::signal<void()> s;
    mock m1;
    mock m2;
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(size_t(0), s.size());
    EXPECT_TRUE(s.connect(&m1, &mock::slot_no_arg));
    EXPECT_TRUE(s.connected(&m1, &mock::slot_no_arg));
    EXPECT_FALSE(s.empty());
    EXPECT_EQ(size_t(1), s.size());
    EXPECT_CALL(m1, slot_no_arg()).Times(2);
    s.emit();

    EXPECT_TRUE(s.connect(&m2, &mock::slot_no_arg));
    EXPECT_TRUE(s.connected(&m2, &mock::slot_no_arg));
    EXPECT_FALSE(s.empty());
    EXPECT_EQ(size_t(2), s.size());
    EXPECT_CALL(m2, slot_no_arg()).Times(1);
    s.emit();
}

TEST(signals, disconnection)
{
    vdk::signal<void()> s;
    mock m1;
    mock m2;
    mock m3;

    EXPECT_TRUE(s.connect(&m1, &mock::slot_no_arg));
    EXPECT_TRUE(s.connected(&m1, &mock::slot_no_arg));
    EXPECT_FALSE(s.empty());
    EXPECT_EQ(size_t(1), s.size());
    EXPECT_CALL(m1, slot_no_arg()).Times(1);

    EXPECT_TRUE(s.connect(&m2, &mock::slot_no_arg));
    EXPECT_TRUE(s.connected(&m2, &mock::slot_no_arg));
    EXPECT_FALSE(s.empty());
    EXPECT_EQ(size_t(2), s.size());
    EXPECT_CALL(m2, slot_no_arg()).Times(1);
    s.emit();

    EXPECT_TRUE(s.disconnect(&m1, &mock::slot_no_arg));
    EXPECT_FALSE(s.empty());
    EXPECT_EQ(size_t(1), s.size());

    EXPECT_TRUE(s.connect(&m3, &mock::slot_no_arg));
    EXPECT_TRUE(s.connected(&m3, &mock::slot_no_arg));
    EXPECT_FALSE(s.empty());
    EXPECT_EQ(size_t(2), s.size());
    EXPECT_CALL(m3, slot_no_arg()).Times(0);

    EXPECT_TRUE(s.disconnect(&m3, &mock::slot_no_arg));
    EXPECT_TRUE(s.disconnect(&m2, &mock::slot_no_arg));
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(size_t(0), s.size());
    s.emit();
}

TEST(signals, disconnect_all)
{
    vdk::signal<void()> s;
    mock m1;
    mock m2;
    EXPECT_TRUE(s.connect(&m1, &mock::slot_no_arg));
    EXPECT_TRUE(s.connected(&m1, &mock::slot_no_arg));
    EXPECT_CALL(m1, slot_no_arg()).Times(0);

    EXPECT_TRUE(s.connect(&m2, &mock::slot_no_arg));
    EXPECT_TRUE(s.connected(&m2, &mock::slot_no_arg));
    EXPECT_CALL(m2, slot_no_arg()).Times(0);

    EXPECT_FALSE(s.empty());
    EXPECT_EQ(size_t(2), s.size());

    s.disconnect_all();
    s.emit();
}

TEST(signals, call_operator_to_emit)
{
    vdk::signal<void()> s;
    mock m1;
    s.connect(&m1, &mock::slot_no_arg);
    EXPECT_CALL(m1, slot_no_arg()).Times(2);
    s.emit();
    s();
}

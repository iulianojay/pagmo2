#include <cassert>
#include <iostream>

#include <pagmo/reflection.hpp>

using namespace pagmo::detail;

enum Color { red, green, blue };

int test1()
{
    constexpr auto r = ^^int;
    typename[:r:] x = 42;       // Same as: int x = 42;
    typename[:^^char:] c = '*'; // Same as: char c = '*';

    static_assert(std::same_as<decltype(x), int>);
    static_assert(std::same_as<decltype(c), char>);
    assert(x == 42);
    assert(c == '*');
    return 0;
}

int test2()
{
    std::cout << '\n';
    std::cout << "enum_to_string(Color::red): " << enum_to_string(Color::red) << '\n';
    std::cout << '\n';
    return 0;
}

int test3()
{
    static_assert(string_to_enum<Color>("red").has_value());
    static_assert(string_to_enum<Color>("red").value() == Color::red);
    static_assert(string_to_enum<Color>(enum_to_string(Color::red)).value() == Color::red);
    return 0;
}

struct A {
    int foo()
    {
        counter++;
        return 42;
    }
    double bar()
    {
        counter++;
        return 7.0;
    }
    int counter_value() const
    {
        return counter;
    }

private:
    int counter = 0;
};

struct B {
    int foo()
    {
        return 99;
    }
    double bar()
    {
        return 3.0;
    }
};

struct DefaultAnyFoo {
    int counter_value() const
    {
        return -1;
    }
};

// Type-erased container built with reflection.
// Stores any type that has foo()->int and bar()->int,
// wiring them up automatically by name rather than via virtual dispatch.
struct AnyFooBar {
    template <typename T>
    constexpr AnyFooBar(T &t)
    {
        std::tie(foo_, bar_, counter_value_) = reflect_functions<DefaultAnyFoo, "foo", "bar", "counter_value">(t);
    }

    int foo()
    {
        return foo_();
    }
    double bar()
    {
        return bar_();
    }
    int counter_value() const
    {
        return counter_value_();
    }

private:
    std::function<int()> foo_;
    std::function<double()> bar_;
    std::function<int()> counter_value_;

    static int default_counter_value()
    {
        return -1; // Indicates no counter_value() function was found
    }
};

int type_erasure_test()
{
    A a_instance;
    AnyFooBar a{a_instance};
    std::cout << "a.foo() = " << a.foo() << '\n';
    std::cout << "a.bar() = " << a.bar() << '\n';
    std::cout << "a_instance.counter_value() = " << a_instance.counter_value() << '\n';
    std::cout << "a.counter_value() = " << a.counter_value() << '\n' << '\n';

    B b_instance;
    AnyFooBar b{b_instance};
    std::cout << "b.foo() = " << b.foo() << '\n';
    std::cout << "b.bar() = " << b.bar() << '\n';
    std::cout << "b.counter_value() = " << b.counter_value() << '\n' << '\n';

    return 0;
}

int function_pack()
{
    A a_instance;
    auto [foo, bar] = reflect_functions<DefaultAnyFoo, "foo", "bar">(a_instance);
    std::cout << "foo() = " << foo() << '\n';
    std::cout << "bar() = " << bar() << '\n';
    std::cout << "counter after pack calls: " << a_instance.counter_value() << '\n';
    return 0;
}

int main()
{
    test1();
    test2();
    test3();
    type_erasure_test();
    function_pack();
}
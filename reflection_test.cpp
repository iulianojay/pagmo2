#include <any>
#include <cassert>
#include <concepts>
#include <functional>
#include <iostream>
#include <meta>
#include <string>
#include <type_traits>

template <typename E>
    requires std::is_enum_v<E>
constexpr std::string enum_to_string(E value)
{
    std::string result = "<unnamed>";
    template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^E)))
    {
        if (value == [:e:]) {
            result = std::meta::identifier_of(e);
        }
    }
    return result;
}

template <typename E>
    requires std::is_enum_v<E>
constexpr std::optional<E> string_to_enum(std::string_view name)
{
    template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^E)))
    {
        if (name == std::meta::identifier_of(e)) {
            return [:e:];
        }
    }

    return std::nullopt;
}

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
    int bar()
    {
        counter++;
        return 7;
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
    int bar()
    {
        return 3;
    }
};

// Type-erased container built with reflection.
// Stores any type that has foo()->int and bar()->int,
// wiring them up automatically by name rather than via virtual dispatch.
struct AnyFooBar {
    template <typename T>
    constexpr AnyFooBar(T &t)
    {
        // Use reflection to find methods named "foo" and "bar" and capture them.
        template for (constexpr auto m :
                      std::define_static_array(std::meta::members_of(^^T, std::meta::access_context::current())))
        {
            if constexpr (std::meta::is_function(m) && std::meta::has_identifier(m)) {
                if constexpr (std::meta::identifier_of(m) == "foo") {
                    foo_ = [&t]() mutable { return t.[:m:](); };
                }
                if constexpr (std::meta::identifier_of(m) == "bar") {
                    bar_ = [&t]() mutable { return t.[:m:](); };
                }
                if constexpr (std::meta::identifier_of(m) == "counter_value") {
                    counter_value_ = [&t]() mutable { return t.[:m:](); };
                }
            }
        }
    }

    int foo()
    {
        return foo_();
    }
    int bar()
    {
        return bar_();
    }
    int counter_value() const
    {
        // This method is only meaningful if the underlying type has a counter_value() method.
        // We can use reflection to check for it and call it if it exists.
        if (counter_value_) {
            return counter_value_();
        }
        return -1; // Indicate that counter_value is not available.
    }

private:
    std::function<int()> foo_;
    std::function<int()> bar_;
    std::function<int()> counter_value_;
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
    std::cout << "b.counter_value() = " << b.counter_value() << '\n';

    return 0;
}

int main()
{
    test1();
    test2();
    test3();
    type_erasure_test();
}
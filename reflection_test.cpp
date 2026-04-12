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

// fixed_string is structural (all-public), usable as an NTTP.
template <std::size_t N>
struct fixed_string {
    char data[N]{};
    constexpr fixed_string(const char (&s)[N])
    {
        std::copy_n(s, N, data);
    }
    constexpr operator std::string_view() const
    {
        return {data, N - 1};
    }
};
template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N>;

template <fixed_string name, typename T>
consteval bool has_member_named()
{
    for (auto m : std::meta::members_of(^^T, std::meta::access_context::current())) {
        if (std::meta::is_function(m) && std::meta::has_identifier(m)
            && std::meta::identifier_of(m) == std::string_view(name))
            return true;
    }
    return false;
}

template <fixed_string name, typename T>
constexpr auto reflect_function_impl(T &t)
{
    template for (constexpr auto m :
                  std::define_static_array(std::meta::members_of(^^T, std::meta::access_context::current())))
    {
        if constexpr (std::meta::is_function(m) && std::meta::has_identifier(m)
                      && std::meta::identifier_of(m) == std::string_view(name)) {
            return [&t]() mutable { return t.[:m:](); };
        }
    }
}

template <fixed_string name, typename T>
constexpr auto reflect_function_impl(T &&t)
{
    template for (constexpr auto m :
                  std::define_static_array(std::meta::members_of(^^T, std::meta::access_context::current())))
    {
        if constexpr (std::meta::is_function(m) && std::meta::has_identifier(m)
                      && std::meta::identifier_of(m) == std::string_view(name)) {
            return [t]() mutable { return t.[:m:](); };
        }
    }
}

template <typename Default_T, fixed_string name, typename T>
constexpr auto reflect_function(T &t)
{
    if constexpr (has_member_named<name, T>()) {
        return reflect_function_impl<name>(t);
    } else {
        return reflect_function_impl<name>(Default_T{}); // Fallback to default type if member not found in T
    }
}

template <typename Default_T, fixed_string... names, typename T>
constexpr auto reflect_functions(T &t)
{
    return std::tuple{reflect_function<Default_T, names>(t)...};
}

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
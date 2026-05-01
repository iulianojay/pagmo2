#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>

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

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(CEREAL_NVP(counter));
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

// External serialization dispatcher: calls T::serialize if present,
// otherwise falls back to DefaultFooBar::serialize (a no-op).
// New archive types are supported just by adding overloads or using
// this template directly.
template <typename Default_T, typename T, typename Archive>
void serialize(T &t, Archive &ar)
{
    if constexpr (has_function_template_named<T, "serialize">()) {
        ar(t);
    } else {
        Default_T fallback{};
        ar(fallback);
    }
}

struct NullFooBar {
    int foo()
    {
        return 0;
    }
    double bar()
    {
        return 0.0;
    }
};

struct DefaultFooBar {
    int counter_value() const
    {
        return -1;
    }

private:
    friend class cereal::access;
    template <class Archive>
    void serialize(Archive &)
    {
    }
};

// struct FooBarInnerBase {
//     virtual ~FooBarInnerBase() = default;
//     virtual int foo() = 0;
//     virtual double bar() = 0;
//     virtual int counter_value() const = 0;
// };

// template <typename T>
// struct FooBarInner : FooBarInnerBase {

//     consteval
//     {
//         auto specs = get_functions_metadata<T, DefaultFooBar, "foo", "bar", "counter_value">();
//         std::meta::define_aggregate(^^FooBarInner, specs);

//         // Pseudocode — requires P3294 injection
//         template for (constexpr auto m : std::meta::members_of(^^T))
//         {
//             queue_injection(^{
//                 [ : return_type_of(m) : ][:identifier_of(m):]([:params_of(m):])
//                     override { return m_value.[:identifier_of(m):]([:forward_params_of(m):]); }});
//         }
//     }

//     int foo() override
//     {
//         return (consteval auto get_function_metadata<T, DefaultFooBar, "foo">())();
//     }

//     double bar() override
//     {
//         return (consteval auto get_function_metadata<T, DefaultFooBar, "bar">())();
//     }

//     int counter_value() const override
//     {
//         return (consteval auto get_function_metadata<T, DefaultFooBar, "counter_value">())();
//     }
// };

// Type-erased container built with reflection.
// Stores any type that has foo()->int and bar()->int,
// wiring them up automatically by name rather than via virtual dispatch.
// Serialization is forwarded through serialize_any_foo_bar; one
// std::function is captured per supported archive type at construction.

struct AnyFooBar {

    struct ValueContainerBase {
    };
    struct FunctionContainer;

    template <typename T = NullFooBar>
    constexpr AnyFooBar(T &&t)
    {
        struct ValueContainer;
        consteval
        {
            std::vector<std::meta::info> specs;
            specs.push_back(std::meta::base_class_spec(^^ValueContainerBase));
            template for (constexpr auto m : std::meta::nonstatic_data_members_of(^^std::remove_cvref_t<T>,
                                                                                  std::meta::access_context::current()))
            {
                specs.push_back(data_member_spec(add_pointer(type_of(m)), {.name = identifier_of(m)}));
            }
            std::meta::define_aggregate(^^ValueContainer, specs);
        }
        values = std::make_unique<ValueContainer>();

        std::tie(foo, bar, counter_value) = wire_functions<DefaultFooBar, "foo", "bar", "counter_value">(t);
    }

    std::function<int()> foo;
    std::function<double()> bar;
    std::function<int()> counter_value;

private:
    std::unique_ptr<ValueContainerBase> values;
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
    auto [foo, bar] = wire_functions<DefaultFooBar, "foo", "bar">(a_instance);
    std::cout << "foo() = " << foo() << '\n';
    std::cout << "bar() = " << bar() << '\n';
    std::cout << "counter after pack calls: " << a_instance.counter_value() << '\n';
    return 0;
}

// int serialization_test()
// {
//     // --- JSON: A has serialize, counter state should round-trip ---
//     A a_instance;
//     AnyFooBar a{a_instance};
//     a.foo(); // counter -> 1
//     a.foo(); // counter -> 2
//     a.bar(); // counter -> 3
//     assert(a.counter_value() == 3);

//     std::string saved_json;
//     {
//         std::ostringstream ss;
//         {
//             cereal::JSONOutputArchive ar(ss);
//             a.save(ar);
//         }
//         saved_json = ss.str();
//     }
//     std::cout << "A saved JSON: " << saved_json << '\n';

//     A a_fresh;
//     AnyFooBar a2{a_fresh};
//     assert(a2.counter_value() == 0);
//     {
//         std::istringstream ss(saved_json);
//         cereal::JSONInputArchive ar(ss);
//         a2.load(ar);
//     }
//     assert(a2.counter_value() == 3);
//     // foo/bar must still dispatch through a2 to a_fresh and mutate its counter
//     assert(a2.foo() == 42);  // counter -> 4
//     assert(a2.bar() == 7.0); // counter -> 5
//     assert(a2.counter_value() == 5);
//     std::cout << "A JSON round-trip passed: counter_value = " << a2.counter_value() << '\n';

//     // --- Binary: same object, different archive type ---
//     std::string saved_bin;
//     {
//         std::ostringstream ss;
//         {
//             cereal::BinaryOutputArchive ar(ss);
//             a.save(ar);
//         }
//         saved_bin = ss.str();
//     }

//     A a_fresh2;
//     AnyFooBar a3{a_fresh2};
//     assert(a3.counter_value() == 0);
//     {
//         std::istringstream ss(saved_bin);
//         cereal::BinaryInputArchive ar(ss);
//         a3.load(ar);
//     }
//     assert(a3.counter_value() == 3);
//     // foo/bar must still dispatch through a3 to a_fresh2 and mutate its counter
//     assert(a3.foo() == 42);  // counter -> 4
//     assert(a3.bar() == 7.0); // counter -> 5
//     assert(a3.counter_value() == 5);
//     std::cout << "A binary round-trip passed: counter_value = " << a3.counter_value() << '\n';

//     // --- Fallback: B has no serialize, DefaultFooBar no-op is used ---
//     B b_instance;
//     AnyFooBar b{b_instance};
//     {
//         std::ostringstream ss;
//         {
//             cereal::JSONOutputArchive ar(ss);
//             b.save(ar);
//         }
//         std::cout << "B fallback JSON: " << ss.str() << '\n';
//         std::istringstream iss(ss.str());
//         cereal::JSONInputArchive ar(iss);
//         b.load(ar); // no-op, must not crash
//     }
//     // B's foo/bar must still return their values after the no-op round-trip
//     assert(b.foo() == 99);
//     assert(b.bar() == 3.0);
//     assert(b.counter_value() == -1); // falls back to DefaultFooBar
//     std::cout << "B fallback round-trip passed\n";

//     return 0;
// }

int main()
{
    test1();
    test2();
    test3();
    type_erasure_test();
    function_pack();
    // serialization_test();
}
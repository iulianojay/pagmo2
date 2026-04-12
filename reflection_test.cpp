#include <cassert>
#include <iostream>
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

struct DefaultAnyFoo {
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

// External serialization dispatcher: calls T::serialize if present,
// otherwise falls back to DefaultAnyFoo::serialize (a no-op).
// New archive types are supported just by adding overloads or using
// this template directly.
template <typename Default_T, typename T, typename Archive>
void serialize(T &t, Archive &ar)
{
    if constexpr (has_function_template_named<"serialize", T>()) {
        ar(t);
    } else {
        Default_T fallback{};
        ar(fallback);
    }
}

// Type-erased container built with reflection.
// Stores any type that has foo()->int and bar()->int,
// wiring them up automatically by name rather than via virtual dispatch.
// Serialization is forwarded through serialize_any_foo_bar; one
// std::function is captured per supported archive type at construction.
struct AnyFooBar {
    template <typename T>
    explicit AnyFooBar(T &t)
    {
        std::tie(m_foo, m_bar, m_counter_value) = reflect_functions<DefaultAnyFoo, "foo", "bar", "counter_value">(t);

        m_save_json = [&t](cereal::JSONOutputArchive &ar) { serialize<DefaultAnyFoo>(t, ar); };
        m_save_bin = [&t](cereal::BinaryOutputArchive &ar) { serialize<DefaultAnyFoo>(t, ar); };
        m_load_json = [&t](cereal::JSONInputArchive &ar) { serialize<DefaultAnyFoo>(t, ar); };
        m_load_bin = [&t](cereal::BinaryInputArchive &ar) { serialize<DefaultAnyFoo>(t, ar); };
    }

    int foo()
    {
        return m_foo();
    }
    double bar()
    {
        return m_bar();
    }
    int counter_value() const
    {
        return m_counter_value();
    }

private:
    std::function<int()> m_foo;
    std::function<double()> m_bar;
    std::function<int()> m_counter_value;

    std::function<void(cereal::JSONOutputArchive &)> m_save_json;
    std::function<void(cereal::JSONInputArchive &)> m_load_json;
    std::function<void(cereal::BinaryOutputArchive &)> m_save_bin;
    std::function<void(cereal::BinaryInputArchive &)> m_load_bin;

    friend class cereal::access;
    friend int serialization_test();

    template <typename Archive>
    void save(Archive &ar)
    {
        if constexpr (std::derived_from<Archive, cereal::JSONOutputArchive>) {
            m_save_json(ar);
        } else if constexpr (std::derived_from<Archive, cereal::JSONInputArchive>) {
            m_load_json(ar);
        } else if constexpr (std::derived_from<Archive, cereal::BinaryOutputArchive>) {
            m_save_bin(ar);
        } else if constexpr (std::derived_from<Archive, cereal::BinaryInputArchive>) {
            m_load_bin(ar);
        } else {
            static_assert(false, "Unsupported archive type");
        }
    }

    template <typename Archive>
    void load(Archive &ar)
    {
        if constexpr (std::derived_from<Archive, cereal::JSONOutputArchive>) {
            m_save_json(ar);
        } else if constexpr (std::derived_from<Archive, cereal::JSONInputArchive>) {
            m_load_json(ar);
        } else if constexpr (std::derived_from<Archive, cereal::BinaryOutputArchive>) {
            m_save_bin(ar);
        } else if constexpr (std::derived_from<Archive, cereal::BinaryInputArchive>) {
            m_load_bin(ar);
        } else {
            static_assert(false, "Unsupported archive type");
        }
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

int serialization_test()
{
    // --- JSON: A has serialize, counter state should round-trip ---
    A a_instance;
    AnyFooBar a{a_instance};
    a.foo(); // counter -> 1
    a.foo(); // counter -> 2
    a.bar(); // counter -> 3
    assert(a.counter_value() == 3);

    std::string saved_json;
    {
        std::ostringstream ss;
        {
            cereal::JSONOutputArchive ar(ss);
            a.save(ar);
        }
        saved_json = ss.str();
    }
    std::cout << "A saved JSON: " << saved_json << '\n';

    A a_fresh;
    AnyFooBar a2{a_fresh};
    assert(a2.counter_value() == 0);
    {
        std::istringstream ss(saved_json);
        cereal::JSONInputArchive ar(ss);
        a2.load(ar);
    }
    assert(a2.counter_value() == 3);
    std::cout << "A JSON round-trip passed: counter_value = " << a2.counter_value() << '\n';

    // --- Binary: same object, different archive type ---
    std::string saved_bin;
    {
        std::ostringstream ss;
        {
            cereal::BinaryOutputArchive ar(ss);
            a.save(ar);
        }
        saved_bin = ss.str();
    }

    A a_fresh2;
    AnyFooBar a3{a_fresh2};
    assert(a3.counter_value() == 0);
    {
        std::istringstream ss(saved_bin);
        cereal::BinaryInputArchive ar(ss);
        a3.load(ar);
    }
    assert(a3.counter_value() == 3);
    std::cout << "A binary round-trip passed: counter_value = " << a3.counter_value() << '\n';

    // --- Fallback: B has no serialize, DefaultAnyFoo no-op is used ---
    B b_instance;
    AnyFooBar b{b_instance};
    {
        std::ostringstream ss;
        {
            cereal::JSONOutputArchive ar(ss);
            b.save(ar);
        }
        std::cout << "B fallback JSON: " << ss.str() << '\n';
        std::istringstream iss(ss.str());
        cereal::JSONInputArchive ar(iss);
        b.load(ar); // no-op, must not crash
    }
    std::cout << "B fallback round-trip passed\n";

    return 0;
}

int main()
{
    test1();
    test2();
    test3();
    type_erasure_test();
    function_pack();
    serialization_test();
}
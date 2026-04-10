/* Copyright 2017-2021 PaGMO development team

This file is part of the PaGMO library.

The PaGMO library is free software; you can redistribute it and/or modify
it under the terms of either:

  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.

or

  * the GNU General Public License as published by the Free Software
    Foundation; either version 3 of the License, or (at your option) any
    later version.

or both in parallel, as here.

The PaGMO library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the PaGMO library.  If not,
see https://www.gnu.org/licenses/. */

#if defined(_MSC_VER)

// Disable warnings from MSVC.
#pragma warning(disable : 4822)

#endif

#include <gtest/gtest.h>

#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>

#include <pagmo/batch_evaluators/default_bfe.hpp>
#include <pagmo/bfe.hpp>
#include <pagmo/detail/type_name.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/problems/null_problem.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/threading.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>
#include <pagmo/exceptions.hpp>

using namespace pagmo;

using udbfe_func_t = vector_double (*)(const problem &, const vector_double &);

inline vector_double udbfe0(const problem &p, const vector_double &dvs)
{
    return vector_double(p.get_nf() * (dvs.size() / p.get_nx()), .5);
}

TEST(bfe_test, type_traits_tests)
{
    EXPECT_TRUE(IsUdBfe<default_bfe>);
    EXPECT_TRUE(!IsUdBfe<const default_bfe>);
    EXPECT_TRUE(!IsUdBfe<default_bfe &>);
    EXPECT_TRUE(!IsUdBfe<const default_bfe &>);

    EXPECT_TRUE(IsUdBfe<decltype(&udbfe0)>);
    EXPECT_TRUE(IsUdBfe<udbfe_func_t>);

    struct non_udbfe_00 {
    };
    EXPECT_TRUE(!IsUdBfe<non_udbfe_00>);
    EXPECT_TRUE(!HasBfeCallOperator<non_udbfe_00>);

    struct non_udbfe_01 {
        vector_double operator()();
    };
    EXPECT_TRUE(!IsUdBfe<non_udbfe_01>);
    EXPECT_TRUE(!HasBfeCallOperator<non_udbfe_01>);

    struct non_udbfe_02 {
        // NOTE: non-const operator.
        vector_double operator()(const problem &, const vector_double &);
    };
    EXPECT_TRUE(!IsUdBfe<non_udbfe_02>);
    EXPECT_TRUE(!HasBfeCallOperator<non_udbfe_02>);

    struct non_udbfe_03 {
        // NOTE: not def ctible.
        non_udbfe_03() = delete;
        vector_double operator()(const problem &, const vector_double &) const;
    };
    EXPECT_TRUE(!IsUdBfe<non_udbfe_03>);
    EXPECT_TRUE(HasBfeCallOperator<non_udbfe_03>);

    EXPECT_TRUE(IsUdBfe<decltype(&udbfe0)>);
    struct udbfe_00 {
        vector_double operator()(const problem &, const vector_double &) const;
    };
    EXPECT_TRUE(IsUdBfe<udbfe_00>);

    // Test std::function as well.
    EXPECT_TRUE(IsUdBfe<std::function<vector_double(const problem &, const vector_double &)>>);
}

struct udbfe1 {
    vector_double operator()(const problem &, const vector_double &) const
    {
        return vector_double{};
    }
    std::string foo = "hello world";
};

struct udbfe2 {
    udbfe2() = default;
    udbfe2(const udbfe2 &other) : foo{new std::string{*other.foo}} {}
    udbfe2(udbfe2 &&) = default;
    vector_double operator()(const problem &, const vector_double &) const
    {
        return vector_double{};
    }
    std::string get_name() const
    {
        return "frobniz";
    }
    thread_safety get_thread_safety() const
    {
        return thread_safety::constant;
    }
    std::unique_ptr<std::string> foo = std::unique_ptr<std::string>{new std::string{"hello world"}};
};

TEST(bfe_test, basic_tests)
{
    bfe bfe0;
    problem p;

    // Public methods.
    EXPECT_TRUE(bfe0.extract<default_bfe>() != nullptr);
    EXPECT_TRUE(bfe0.extract<udbfe_func_t>() == nullptr);
    EXPECT_TRUE(static_cast<const bfe &>(bfe0).extract<default_bfe>() != nullptr);
    EXPECT_TRUE(static_cast<const bfe &>(bfe0).extract<udbfe_func_t>() == nullptr);
    EXPECT_TRUE(bfe0.is<default_bfe>());
    EXPECT_TRUE(!bfe0.is<udbfe_func_t>());
    EXPECT_TRUE(bfe0.get_name() == "Default batch fitness evaluator");
    EXPECT_TRUE(bfe{udbfe1{}}.get_name() == detail::type_name<udbfe1>());
    EXPECT_TRUE(bfe0.get_extra_info().empty());
    EXPECT_TRUE(bfe0.get_thread_safety() == thread_safety::basic);

    // Constructors, assignments.
    bfe bfe1{udbfe0};
    EXPECT_TRUE(bfe1.is<udbfe_func_t>());
    EXPECT_TRUE(*bfe1.extract<udbfe_func_t>() == udbfe0);
    // Generic constructor with copy.
    udbfe1 b1;
    bfe bfe2{b1};
    EXPECT_TRUE(b1.foo == "hello world");
    EXPECT_TRUE(bfe2.extract<udbfe1>()->foo == "hello world");
    // Generic constructor with move.
    udbfe2 b2;
    bfe bfe3{std::move(b2)};
    EXPECT_TRUE(b2.foo.get() == nullptr);
    EXPECT_TRUE(bfe3.extract<udbfe2>()->foo.get() != nullptr);
    EXPECT_TRUE(*bfe3.extract<udbfe2>()->foo == "hello world");
    // Copy constructor.
    udbfe2 b3;
    bfe bfe4{b3}, bfe5{bfe4};
    EXPECT_TRUE(*bfe5.extract<udbfe2>()->foo == "hello world");
    EXPECT_TRUE(bfe5.extract<udbfe2>()->foo.get() != bfe4.extract<udbfe2>()->foo.get());
    EXPECT_TRUE(bfe5(p, vector_double{}) == vector_double{});
    EXPECT_TRUE(bfe5.get_name() == "frobniz");
    EXPECT_TRUE(bfe5.get_thread_safety() == thread_safety::constant);
    // Move constructor.
    bfe bfe6{std::move(bfe5)};
    EXPECT_TRUE(*bfe6.extract<udbfe2>()->foo == "hello world");
    EXPECT_TRUE(bfe6(p, vector_double{}) == vector_double{});
    EXPECT_TRUE(bfe6.get_name() == "frobniz");
    EXPECT_TRUE(bfe6.get_thread_safety() == thread_safety::constant);
    // Revive bfe5 via copy assignment.
    bfe5 = bfe6;
    EXPECT_TRUE(*bfe5.extract<udbfe2>()->foo == "hello world");
    EXPECT_TRUE(bfe5(p, vector_double{}) == vector_double{});
    EXPECT_TRUE(bfe5.get_name() == "frobniz");
    EXPECT_TRUE(bfe5.get_thread_safety() == thread_safety::constant);
    // Revive bfe5 via move assignment.
    bfe bfe7{std::move(bfe5)};
    bfe5 = std::move(bfe6);
    EXPECT_TRUE(*bfe5.extract<udbfe2>()->foo == "hello world");
    EXPECT_TRUE(bfe5(p, vector_double{}) == vector_double{});
    EXPECT_TRUE(bfe5.get_name() == "frobniz");
    EXPECT_TRUE(bfe5.get_thread_safety() == thread_safety::constant);
    // Self move-assignment.
    bfe5 = std::move(*&bfe5);
    EXPECT_TRUE(*bfe5.extract<udbfe2>()->foo == "hello world");
    EXPECT_TRUE(bfe5(p, vector_double{}) == vector_double{});
    EXPECT_TRUE(bfe5.get_name() == "frobniz");
    EXPECT_TRUE(bfe5.get_thread_safety() == thread_safety::constant);

    // Minimal iostream test.
    {
        std::ostringstream oss;
        oss << bfe0;
        EXPECT_TRUE(!oss.str().empty());
    }

    // Minimal serialization test.
    {
        std::string before;
        std::stringstream ss;
        {
            before = lexical_cast<std::string>(bfe0);
            cereal::BinaryOutputArchive oarchive(ss);
            oarchive(bfe0);
        }
        bfe0 = bfe{udbfe0};
        EXPECT_TRUE(bfe0.is<udbfe_func_t>());
        EXPECT_TRUE(before != lexical_cast<std::string>(bfe0));
        {
            cereal::BinaryInputArchive iarchive(ss);
            iarchive(bfe0);
        }
        EXPECT_TRUE(before == lexical_cast<std::string>(bfe0));
        EXPECT_TRUE(bfe0.is<default_bfe>());
    }
}

TEST(bfe_test, optional_tests)
{
    // get_name().
    struct udbfe_00 {
        vector_double operator()(const problem &, const vector_double &) const
        {
            return vector_double{};
        }
        std::string get_name() const
        {
            return "frobniz";
        }
    };
    EXPECT_EQ(bfe{udbfe_00{}}.get_name(), "frobniz");
    struct udbfe_01 {
        vector_double operator()(const problem &, const vector_double &) const
        {
            return vector_double{};
        }
        // Missing const.
        std::string get_name()
        {
            return "frobniz";
        }
    };
    EXPECT_TRUE(bfe{udbfe_01{}}.get_name() != "frobniz");

    // get_extra_info().
    struct udbfe_02 {
        vector_double operator()(const problem &, const vector_double &) const
        {
            return vector_double{};
        }
        std::string get_extra_info() const
        {
            return "frobniz";
        }
    };
    EXPECT_EQ(bfe{udbfe_02{}}.get_extra_info(), "frobniz");
    struct udbfe_03 {
        vector_double operator()(const problem &, const vector_double &) const
        {
            return vector_double{};
        }
        // Missing const.
        std::string get_extra_info()
        {
            return "frobniz";
        }
    };
    EXPECT_TRUE(bfe{udbfe_03{}}.get_extra_info().empty());

    // get_thread_safety().
    struct udbfe_04 {
        vector_double operator()(const problem &, const vector_double &) const
        {
            return vector_double{};
        }
        thread_safety get_thread_safety() const
        {
            return thread_safety::constant;
        }
    };
    EXPECT_EQ(bfe{udbfe_04{}}.get_thread_safety(), thread_safety::constant);
    struct udbfe_05 {
        vector_double operator()(const problem &, const vector_double &) const
        {
            return vector_double{};
        }
        // Missing const.
        thread_safety get_thread_safety()
        {
            return thread_safety::constant;
        }
    };
    EXPECT_EQ(bfe{udbfe_05{}}.get_thread_safety(), thread_safety::basic);
}

TEST(bfe_test, stream_operator)
{
    struct udbfe_00 {
        vector_double operator()(const problem &, const vector_double &) const
        {
            return vector_double{};
        }
    };
    {
        std::ostringstream oss;
        oss << bfe{udbfe_00{}};
        EXPECT_TRUE(!oss.str().empty());
    }
    struct udbfe_01 {
        vector_double operator()(const problem &, const vector_double &) const
        {
            return vector_double{};
        }
        std::string get_extra_info() const
        {
            return "bartoppo";
        }
    };
    {
        std::ostringstream oss;
        oss << bfe{udbfe_01{}};
        const auto st = oss.str();
        EXPECT_TRUE(st.contains("bartoppo"));
        EXPECT_TRUE(st.contains("Extra info:"));
    }
    std::cout << bfe{} << '\n';
}

TEST(bfe_test, call_operator)
{
    struct udbfe_00 {
        vector_double operator()(const problem &p, const vector_double &dvs) const
        {
            return vector_double(p.get_nf() * (dvs.size() / p.get_nx()), 1.);
        }
    };
    bfe bfe0{udbfe_00{}};
    EXPECT_TRUE(bfe0(problem{}, vector_double{.5}) == vector_double{1.});
    EXPECT_TRUE(bfe0(problem{null_problem{3}}, vector_double{.5}) == (vector_double{1., 1., 1.}));
    // Try with a function.
    bfe bfe0a{udbfe0};
    EXPECT_TRUE(bfe0a(problem{null_problem{3}}, vector_double{.5}) == (vector_double{.5, .5, .5}));
    // Try passing in a wrong dvs.
    EXPECT_THROW(bfe0(problem{rosenbrock{}}, vector_double{.5}), batch_eval_error);
    // Try a udfbe which returns a bogus vector of fitnesses.
    struct udbfe_01 {
        vector_double operator()(const problem &p, const vector_double &dvs) const
        {
            return vector_double(p.get_nf() * (dvs.size() / p.get_nx()) + 1u, 1.);
        }
    };
    bfe bfe1{udbfe_01{}};
    EXPECT_THROW(bfe1(problem{null_problem{3}}, vector_double{.5}), batch_eval_error);
    // Try a udfbe which returns a bogus number of fitnesses.
    struct udbfe_02 {
        vector_double operator()(const problem &p, const vector_double &dvs) const
        {

            return vector_double(p.get_nf() * ((dvs.size() + 1u) / p.get_nx()), 1.);
        }
    };
    bfe bfe2{udbfe_02{}};
    EXPECT_THROW(bfe2(problem{null_problem{3}}, vector_double{.5}), batch_eval_error);
}

struct udbfe_a {
    vector_double operator()(const problem &p, const vector_double &dvs) const
    {
        return vector_double(p.get_nf() * (dvs.size() / p.get_nx()), 1.);
    }
    std::string get_name() const
    {
        return "abba";
    }
    std::string get_extra_info() const
    {
        return "dabba";
    }
    thread_safety get_thread_safety() const
    {
        return thread_safety::constant;
    }
    template <typename Archive>
    void serialize(Archive &ar, unsigned)
    {
        ar & state;
    }
    int state = 42;
};

PAGMO_S11N_BFE_EXPORT(udbfe_a)

// Serialization tests.
TEST(bfe_test, s11n)
{
    bfe bfe0{udbfe_a{}};
    EXPECT_TRUE(bfe0.extract<udbfe_a>()->state == 42);
    bfe0.extract<udbfe_a>()->state = -42;
    // Store the string representation.
    std::stringstream ss;
    auto before = lexical_cast<std::string>(bfe0);
    // Now serialize, deserialize and compare the result.
    {
        cereal::BinaryOutputArchive oarchive(ss);
        oarchive(bfe0);
    }
    // Change the content of p before deserializing.
    bfe0 = bfe{};
    {
        cereal::BinaryInputArchive iarchive(ss);
        iarchive(bfe0);
    }
    auto after = lexical_cast<std::string>(bfe0);
    EXPECT_EQ(before, after);
    EXPECT_TRUE(bfe0.is<udbfe_a>());
    EXPECT_TRUE(bfe0.extract<udbfe_a>()->state = -42);
}

TEST(bfe_test, lambda_std_function)
{
    auto fun = [](const problem &p, const vector_double &dvs) {
        return vector_double(p.get_nf() * (dvs.size() / p.get_nx()), 1.);
    };
    EXPECT_TRUE(!IsUdBfe<decltype(fun)>);
#if !defined(_MSC_VER)
    EXPECT_TRUE(IsUdBfe<decltype(+fun)>);
#endif
    auto stdfun = std::function<vector_double(const problem &, const vector_double &)>(fun);
    EXPECT_TRUE(IsUdBfe<decltype(stdfun)>);

#if !defined(_MSC_VER)
    {
        bfe bfe0{+fun};
        EXPECT_TRUE(bfe0(problem{}, vector_double{.5}) == vector_double{1.});
        EXPECT_TRUE(bfe0(problem{null_problem{3}}, vector_double{.5}) == (vector_double{1., 1., 1.}));
    }
#endif

    {
        bfe bfe0{stdfun};
        EXPECT_TRUE(bfe0(problem{}, vector_double{.5}) == vector_double{1.});
        EXPECT_TRUE(bfe0(problem{null_problem{3}}, vector_double{.5}) == (vector_double{1., 1., 1.}));
    }
}

TEST(bfe_test, is_valid)
{
    bfe p0;
    EXPECT_TRUE(p0.is_valid());
    bfe p1(std::move(p0));
    EXPECT_TRUE(!p0.is_valid());
    p0 = bfe{udbfe_a{}};
    EXPECT_TRUE(p0.is_valid());
    p1 = std::move(p0);
    EXPECT_TRUE(!p0.is_valid());
    p0 = bfe{udbfe_a{}};
    EXPECT_TRUE(p0.is_valid());
}

TEST(bfe_test, generic_assignment)
{
    bfe p0;
    EXPECT_TRUE(p0.is<default_bfe>());
    EXPECT_TRUE(&(p0 = udbfe_a{}) == &p0);
    EXPECT_TRUE(p0.is_valid());
    EXPECT_TRUE(p0.is<udbfe_a>());
    p0 = udbfe0;
    EXPECT_TRUE(p0.is<udbfe_func_t>());
    p0 = &udbfe0;
    EXPECT_TRUE(p0.is<udbfe_func_t>());
    EXPECT_TRUE((!std::is_assignable<bfe, void>::value));
    EXPECT_TRUE((!std::is_assignable<bfe, int &>::value));
    EXPECT_TRUE((!std::is_assignable<bfe, const int &>::value));
    EXPECT_TRUE((!std::is_assignable<bfe, int &&>::value));
}

TEST(bfe_test, type_index)
{
    bfe p0;
    EXPECT_TRUE(p0.get_type_index() == std::type_index(typeid(default_bfe)));
    p0 = bfe{udbfe1{}};
    EXPECT_TRUE(p0.get_type_index() == std::type_index(typeid(udbfe1)));
}

TEST(bfe_test, get_ptr)
{
    bfe p0;
    EXPECT_TRUE(p0.get_ptr() == p0.extract<default_bfe>());
    EXPECT_TRUE(static_cast<const bfe &>(p0).get_ptr() == static_cast<const bfe &>(p0).extract<default_bfe>());
    p0 = bfe{udbfe1{}};
    EXPECT_TRUE(p0.get_ptr() == p0.extract<udbfe1>());
    EXPECT_TRUE(static_cast<const bfe &>(p0).get_ptr() == static_cast<const bfe &>(p0).extract<udbfe1>());
}

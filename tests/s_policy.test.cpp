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

#include <initializer_list>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>

#include <pagmo/detail/type_name.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/s_policies/select_best.hpp>
#include <pagmo/s_policy.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>
#include <pagmo/exceptions.hpp>

using namespace pagmo;

TEST(s_policy_test, type_traits_tests)
{
    EXPECT_TRUE(!IsUdSPolicy<void>);
    EXPECT_TRUE(!IsUdSPolicy<int>);
    EXPECT_TRUE(!IsUdSPolicy<double>);

    struct udsp00 {
        individuals_group_t select(const individuals_group_t &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double &) const;
    };

    EXPECT_TRUE(IsUdSPolicy<udsp00>);
    EXPECT_TRUE(!IsUdSPolicy<const udsp00>);
    EXPECT_TRUE(!IsUdSPolicy<const udsp00 &>);
    EXPECT_TRUE(!IsUdSPolicy<udsp00 &>);

    struct no_udsp00 {
        void select(const individuals_group_t &, const vector_double::size_type &, const vector_double::size_type &,
                    const vector_double::size_type &, const vector_double::size_type &,
                    const vector_double::size_type &, const vector_double &) const;
    };

    EXPECT_TRUE(!IsUdSPolicy<no_udsp00>);

    struct no_udsp01 {
        individuals_group_t select(const individuals_group_t &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double &);
    };

    EXPECT_TRUE(!IsUdSPolicy<no_udsp01>);

    struct no_udsp02 {
        no_udsp02() = delete;
        individuals_group_t select(const individuals_group_t &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double &) const;
    };

    EXPECT_TRUE(!IsUdSPolicy<no_udsp02>);
}

struct udsp1 {
    individuals_group_t select(const individuals_group_t &inds, const vector_double::size_type &,
                               const vector_double::size_type &, const vector_double::size_type &,
                               const vector_double::size_type &, const vector_double::size_type &,
                               const vector_double &) const
    {
        return inds;
    }
    std::string foo = "hello world";
};

struct udsp2 {
    udsp2() = default;
    udsp2(const udsp2 &other) : foo{new std::string{*other.foo}} {}
    udsp2(udsp2 &&) = default;
    individuals_group_t select(const individuals_group_t &inds, const vector_double::size_type &,
                               const vector_double::size_type &, const vector_double::size_type &,
                               const vector_double::size_type &, const vector_double::size_type &,
                               const vector_double &) const
    {
        return inds;
    }
    std::string get_name() const
    {
        return "frobniz";
    }
    std::unique_ptr<std::string> foo = std::unique_ptr<std::string>{new std::string{"hello world"}};
};

TEST(s_policy_test, basic_tests)
{
    s_policy r;

    EXPECT_TRUE(r.is<select_best>());
    EXPECT_TRUE(!r.is<udsp1>());

    EXPECT_TRUE(r.extract<select_best>() != nullptr);
    EXPECT_TRUE(r.extract<udsp1>() == nullptr);

    EXPECT_TRUE(static_cast<const s_policy &>(r).extract<select_best>() != nullptr);
    EXPECT_TRUE(static_cast<const s_policy &>(r).extract<udsp1>() == nullptr);

    EXPECT_TRUE(r.get_name() == "Select best");
    EXPECT_TRUE(!r.get_extra_info().empty());

    EXPECT_TRUE(s_policy(udsp1{}).get_extra_info().empty());
    EXPECT_TRUE(s_policy(udsp1{}).get_name() == detail::type_name<udsp1>());

    // Constructors, assignments.
    // Generic constructor with copy.
    udsp1 r1;
    s_policy s_pol1{r1};
    EXPECT_TRUE(r1.foo == "hello world");
    EXPECT_TRUE(s_pol1.extract<udsp1>()->foo == "hello world");
    // Generic constructor with move.
    udsp2 r2;
    s_policy s_pol2{std::move(r2)};
    EXPECT_TRUE(r2.foo.get() == nullptr);
    EXPECT_TRUE(s_pol2.extract<udsp2>()->foo.get() != nullptr);
    EXPECT_TRUE(*s_pol2.extract<udsp2>()->foo == "hello world");
    // Copy constructor.
    udsp2 r3;
    s_policy s_pol3{r3}, s_pol4{s_pol3};
    EXPECT_TRUE(*s_pol4.extract<udsp2>()->foo == "hello world");
    EXPECT_TRUE(s_pol4.extract<udsp2>()->foo.get() != s_pol3.extract<udsp2>()->foo.get());
    EXPECT_TRUE(s_pol4.get_name() == "frobniz");
    // Move constructor.
    s_policy s_pol5{std::move(s_pol4)};
    EXPECT_TRUE(*s_pol5.extract<udsp2>()->foo == "hello world");
    EXPECT_TRUE(s_pol5.get_name() == "frobniz");
    // Revive s_pol4 via copy assignment.
    s_pol4 = s_pol5;
    EXPECT_TRUE(*s_pol4.extract<udsp2>()->foo == "hello world");
    EXPECT_TRUE(s_pol4.get_name() == "frobniz");
    // Revive s_pol4 via move assignment.
    s_policy s_pol6{std::move(s_pol4)};
    s_pol4 = std::move(s_pol5);
    EXPECT_TRUE(*s_pol4.extract<udsp2>()->foo == "hello world");
    EXPECT_TRUE(s_pol4.get_name() == "frobniz");
    // Self move-assignment.
    s_pol4 = std::move(*&s_pol4);
    EXPECT_TRUE(*s_pol4.extract<udsp2>()->foo == "hello world");
    EXPECT_TRUE(s_pol4.get_name() == "frobniz");

    // Minimal iostream test.
    {
        std::ostringstream oss;
        oss << r;
        EXPECT_TRUE(!oss.str().empty());
    }

    // Minimal serialization test.
    {
        std::string before;
        std::stringstream ss;
        {
            before = lexical_cast<std::string>(r);
            cereal::BinaryOutputArchive oarchive(ss);
            oarchive(r);
        }
        r = s_policy{udsp1{}};
        EXPECT_TRUE(r.is<udsp1>());
        EXPECT_TRUE(before != lexical_cast<std::string>(r));
        {
            cereal::BinaryInputArchive iarchive(ss);
            iarchive(r);
        }
        EXPECT_TRUE(before == lexical_cast<std::string>(r));
        EXPECT_TRUE(r.is<select_best>());
    }

    std::cout << s_policy{} << '\n';
}

TEST(s_policy_test, optional_tests)
{
    // get_name().
    struct udsp_00 {
        individuals_group_t select(const individuals_group_t &inds, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double &) const
        {
            return inds;
        }
        std::string get_name() const
        {
            return "frobniz";
        }
    };
    EXPECT_EQ(s_policy{udsp_00{}}.get_name(), "frobniz");
    struct udsp_01 {
        individuals_group_t select(const individuals_group_t &inds, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double &) const
        {
            return inds;
        }
        // Missing const.
        std::string get_name()
        {
            return "frobniz";
        }
    };
    EXPECT_TRUE(s_policy{udsp_01{}}.get_name() != "frobniz");

    // get_extra_info().
    struct udsp_02 {
        individuals_group_t select(const individuals_group_t &inds, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double &) const
        {
            return inds;
        }
        std::string get_extra_info() const
        {
            return "frobniz";
        }
    };
    EXPECT_EQ(s_policy{udsp_02{}}.get_extra_info(), "frobniz");
    struct udsp_03 {
        individuals_group_t select(const individuals_group_t &inds, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double &) const
        {
            return inds;
        }
        // Missing const.
        std::string get_extra_info()
        {
            return "frobniz";
        }
    };
    EXPECT_TRUE(s_policy{udsp_03{}}.get_extra_info().empty());
}

TEST(s_policy_test, stream_operator)
{
    struct udsp_00 {
        individuals_group_t select(const individuals_group_t &inds, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double &) const
        {
            return inds;
        }
    };
    {
        std::ostringstream oss;
        oss << s_policy{udsp_00{}};
        EXPECT_TRUE(!oss.str().empty());
    }
    struct udsp_01 {
        individuals_group_t select(const individuals_group_t &inds, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double &) const
        {
            return inds;
        }
        std::string get_extra_info() const
        {
            return "bartoppo";
        }
    };
    {
        std::ostringstream oss;
        oss << s_policy{udsp_01{}};
        const auto st = oss.str();
        EXPECT_TRUE(st.contains("bartoppo"));
        EXPECT_TRUE(st.contains("Extra info:"));
    }
}

TEST(s_policy_test, selection)
{
    s_policy r0;

    EXPECT_THROW(r0.select(individuals_group_t{{0}, {}, {}}, 0, 0, 0, 0, 0, {}), policy_config_error);

    EXPECT_THROW(r0.select(individuals_group_t{{0}, {{1.}}, {{1.}}}, 0, 0, 0, 0, 0, {}), policy_config_error);

    EXPECT_THROW(r0.select(individuals_group_t{{0}, {{1.}}, {{1.}}}, 1, 2, 0, 0, 0, {}), policy_config_error);

    EXPECT_THROW(r0.select(individuals_group_t{{0}, {{1.}}, {{1.}}}, 1, 0, 0, 0, 0, {}), policy_config_error);

    EXPECT_THROW(r0.select(individuals_group_t{{0}, {{1.}}, {{1.}}}, 1, 0,
                           std::numeric_limits<vector_double::size_type>::max(), 0, 0, {}), policy_config_error);

    EXPECT_THROW(r0.select(individuals_group_t{{0}, {{1.}}, {{1.}}}, 1, 0, 1,
                           std::numeric_limits<vector_double::size_type>::max(), 0, {}), policy_config_error);

    EXPECT_THROW(r0.select(individuals_group_t{{0}, {{1.}}, {{1.}}}, 1, 0, 1, 0,
                           std::numeric_limits<vector_double::size_type>::max(), {}), policy_config_error);

    EXPECT_THROW(r0.select(individuals_group_t{{0}, {{1.}}, {{1.}}}, 1, 0, 1, 1, 1, {}), policy_config_error);

    EXPECT_THROW(r0.select(individuals_group_t{{0, 1}, {{1.}, {}}, {{1.}, {1.}}}, 1, 0, 1, 0, 0, {}), policy_config_error);

    EXPECT_THROW(r0.select(individuals_group_t{{0, 1}, {{1.}, {1.}}, {{1.}, {}}}, 1, 0, 1, 0, 0, {}), policy_config_error);

    struct fail_0 {
        individuals_group_t select(const individuals_group_t &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double &) const
        {
            return individuals_group_t{{0}, {}, {}};
        }
        std::string get_name() const
        {
            return "fail_0";
        }
    };

    EXPECT_THROW(s_policy{fail_0{}}.select(individuals_group_t{{0, 1}, {{1.}, {1.}}, {{1.}, {1.}}}, 1, 0, 1, 0, 0, {}), policy_config_error);

    struct fail_1 {
        individuals_group_t select(const individuals_group_t &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double &) const
        {
            return individuals_group_t{{0, 1}, {{1}, {}}, {{1}, {1}}};
        }
        std::string get_name() const
        {
            return "fail_1";
        }
    };

    EXPECT_THROW(s_policy{fail_1{}}.select(individuals_group_t{{0, 1}, {{1.}, {1.}}, {{1.}, {1.}}}, 1, 0, 1, 0, 0, {}), policy_config_error);

    struct fail_2 {
        individuals_group_t select(const individuals_group_t &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double::size_type &, const vector_double::size_type &,
                                   const vector_double &) const
        {
            return individuals_group_t{{0, 1}, {{1}, {1}}, {{1}, {}}};
        }
        std::string get_name() const
        {
            return "fail_2";
        }
    };

    EXPECT_THROW(s_policy{fail_2{}}.select(individuals_group_t{{0, 1}, {{1.}, {1.}}, {{1.}, {1.}}}, 1, 0, 1, 0, 0, {}), policy_config_error);
}

struct udsp_a {
    individuals_group_t select(const individuals_group_t &inds, const vector_double::size_type &,
                               const vector_double::size_type &, const vector_double::size_type &,
                               const vector_double::size_type &, const vector_double::size_type &,
                               const vector_double &) const
    {
        return inds;
    }
    std::string get_name() const
    {
        return "abba";
    }
    std::string get_extra_info() const
    {
        return "dabba";
    }
    template <typename Archive>
    void serialize(Archive &ar, unsigned)
    {
        ar & state;
    }
    int state = 42;
};

PAGMO_S11N_S_POLICY_EXPORT(udsp_a)

// Serialization tests.
TEST(s_policy_test, s11n)
{
    s_policy s_pol0{udsp_a{}};
    EXPECT_TRUE(s_pol0.extract<udsp_a>()->state == 42);
    s_pol0.extract<udsp_a>()->state = -42;
    // Store the string representation.
    std::stringstream ss;
    auto before = lexical_cast<std::string>(s_pol0);
    // Now serialize, deserialize and compare the result.
    {
        cereal::BinaryOutputArchive oarchive(ss);
        oarchive(s_pol0);
    }
    // Change the content of p before deserializing.
    s_pol0 = s_policy{};
    {
        cereal::BinaryInputArchive iarchive(ss);
        iarchive(s_pol0);
    }
    auto after = lexical_cast<std::string>(s_pol0);
    EXPECT_EQ(before, after);
    EXPECT_TRUE(s_pol0.is<udsp_a>());
    EXPECT_TRUE(s_pol0.extract<udsp_a>()->state = -42);
}

TEST(s_policy_test, is_valid)
{
    s_policy p0;
    EXPECT_TRUE(p0.is_valid());
    s_policy p1(std::move(p0));
    EXPECT_TRUE(!p0.is_valid());
    p0 = s_policy{udsp_a{}};
    EXPECT_TRUE(p0.is_valid());
    p1 = std::move(p0);
    EXPECT_TRUE(!p0.is_valid());
    p0 = s_policy{udsp_a{}};
    EXPECT_TRUE(p0.is_valid());
}

TEST(s_policy_test, generic_assignment)
{
    s_policy p0;
    EXPECT_TRUE(p0.is<select_best>());
    EXPECT_TRUE(&(p0 = udsp_a{}) == &p0);
    EXPECT_TRUE(p0.is_valid());
    EXPECT_TRUE(p0.is<udsp_a>());
    p0 = udsp1{};
    EXPECT_TRUE(p0.is<udsp1>());
    EXPECT_TRUE((!std::is_assignable<s_policy, void>::value));
    EXPECT_TRUE((!std::is_assignable<s_policy, int &>::value));
    EXPECT_TRUE((!std::is_assignable<s_policy, const int &>::value));
    EXPECT_TRUE((!std::is_assignable<s_policy, int &&>::value));
}

TEST(s_policy_test, type_index)
{
    s_policy p0;
    EXPECT_TRUE(p0.get_type_index() == std::type_index(typeid(select_best)));
    p0 = s_policy{udsp1{}};
    EXPECT_TRUE(p0.get_type_index() == std::type_index(typeid(udsp1)));
}

TEST(s_policy_test, get_ptr)
{
    s_policy p0;
    EXPECT_TRUE(p0.get_ptr() == p0.extract<select_best>());
    EXPECT_TRUE(static_cast<const s_policy &>(p0).get_ptr()
                == static_cast<const s_policy &>(p0).extract<select_best>());
    p0 = s_policy{udsp1{}};
    EXPECT_TRUE(p0.get_ptr() == p0.extract<udsp1>());
    EXPECT_TRUE(static_cast<const s_policy &>(p0).get_ptr() == static_cast<const s_policy &>(p0).extract<udsp1>());
}

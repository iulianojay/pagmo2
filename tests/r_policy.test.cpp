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
#include <pagmo/r_policies/fair_replace.hpp>
#include <pagmo/r_policy.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>
#include <pagmo/exceptions.hpp>

using namespace pagmo;

TEST(r_policy_test, type_traits_tests)
{
    EXPECT_TRUE(!IsUdRPolicy<void>);
    EXPECT_TRUE(!IsUdRPolicy<int>);
    EXPECT_TRUE(!IsUdRPolicy<double>);

    struct udrp00 {
        individuals_group_t replace(const individuals_group_t &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double &, const individuals_group_t &) const;
    };

    EXPECT_TRUE(IsUdRPolicy<udrp00>);
    EXPECT_TRUE(!IsUdRPolicy<const udrp00>);
    EXPECT_TRUE(!IsUdRPolicy<const udrp00 &>);
    EXPECT_TRUE(!IsUdRPolicy<udrp00 &>);

    struct no_udrp00 {
        void replace(const individuals_group_t &, const vector_double::size_type &, const vector_double::size_type &,
                     const vector_double::size_type &, const vector_double::size_type &,
                     const vector_double::size_type &, const vector_double &, const individuals_group_t &) const;
    };

    EXPECT_TRUE(!IsUdRPolicy<no_udrp00>);

    struct no_udrp01 {
        individuals_group_t replace(const individuals_group_t &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double &, const individuals_group_t &);
    };

    EXPECT_TRUE(!IsUdRPolicy<no_udrp01>);

    struct no_udrp02 {
        no_udrp02() = delete;
        individuals_group_t replace(const individuals_group_t &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double &, const individuals_group_t &) const;
    };

    EXPECT_TRUE(!IsUdRPolicy<no_udrp02>);
}

struct udrp1 {
    individuals_group_t replace(const individuals_group_t &inds, const vector_double::size_type &,
                                const vector_double::size_type &, const vector_double::size_type &,
                                const vector_double::size_type &, const vector_double::size_type &,
                                const vector_double &, const individuals_group_t &) const
    {
        return inds;
    }
    std::string foo = "hello world";
};

struct udrp2 {
    udrp2() = default;
    udrp2(const udrp2 &other) : foo{new std::string{*other.foo}} {}
    udrp2(udrp2 &&) = default;
    individuals_group_t replace(const individuals_group_t &inds, const vector_double::size_type &,
                                const vector_double::size_type &, const vector_double::size_type &,
                                const vector_double::size_type &, const vector_double::size_type &,
                                const vector_double &, const individuals_group_t &) const
    {
        return inds;
    }
    std::string get_name() const
    {
        return "frobniz";
    }
    std::unique_ptr<std::string> foo = std::unique_ptr<std::string>{new std::string{"hello world"}};
};

TEST(r_policy_test, basic_tests)
{
    r_policy r;

    EXPECT_TRUE(r.is<fair_replace>());
    EXPECT_TRUE(!r.is<udrp1>());

    EXPECT_TRUE(r.extract<fair_replace>() != nullptr);
    EXPECT_TRUE(r.extract<udrp1>() == nullptr);

    EXPECT_TRUE(static_cast<const r_policy &>(r).extract<fair_replace>() != nullptr);
    EXPECT_TRUE(static_cast<const r_policy &>(r).extract<udrp1>() == nullptr);

    EXPECT_TRUE(r.get_name() == "Fair replace");
    EXPECT_TRUE(!r.get_extra_info().empty());

    EXPECT_TRUE(r_policy(udrp1{}).get_extra_info().empty());
    EXPECT_TRUE(r_policy(udrp1{}).get_name() == detail::type_name<udrp1>());

    // Constructors, assignments.
    // Generic constructor with copy.
    udrp1 r1;
    r_policy r_pol1{r1};
    EXPECT_TRUE(r1.foo == "hello world");
    EXPECT_TRUE(r_pol1.extract<udrp1>()->foo == "hello world");
    // Generic constructor with move.
    udrp2 r2;
    r_policy r_pol2{std::move(r2)};
    EXPECT_TRUE(r2.foo.get() == nullptr);
    EXPECT_TRUE(r_pol2.extract<udrp2>()->foo.get() != nullptr);
    EXPECT_TRUE(*r_pol2.extract<udrp2>()->foo == "hello world");
    // Copy constructor.
    udrp2 r3;
    r_policy r_pol3{r3}, r_pol4{r_pol3};
    EXPECT_TRUE(*r_pol4.extract<udrp2>()->foo == "hello world");
    EXPECT_TRUE(r_pol4.extract<udrp2>()->foo.get() != r_pol3.extract<udrp2>()->foo.get());
    EXPECT_TRUE(r_pol4.get_name() == "frobniz");
    // Move constructor.
    r_policy r_pol5{std::move(r_pol4)};
    EXPECT_TRUE(*r_pol5.extract<udrp2>()->foo == "hello world");
    EXPECT_TRUE(r_pol5.get_name() == "frobniz");
    // Revive r_pol4 via copy assignment.
    r_pol4 = r_pol5;
    EXPECT_TRUE(*r_pol4.extract<udrp2>()->foo == "hello world");
    EXPECT_TRUE(r_pol4.get_name() == "frobniz");
    // Revive r_pol4 via move assignment.
    r_policy r_pol6{std::move(r_pol4)};
    r_pol4 = std::move(r_pol5);
    EXPECT_TRUE(*r_pol4.extract<udrp2>()->foo == "hello world");
    EXPECT_TRUE(r_pol4.get_name() == "frobniz");
    // Self move-assignment.
    r_pol4 = std::move(*&r_pol4);
    EXPECT_TRUE(*r_pol4.extract<udrp2>()->foo == "hello world");
    EXPECT_TRUE(r_pol4.get_name() == "frobniz");

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
        r = r_policy{udrp1{}};
        EXPECT_TRUE(r.is<udrp1>());
        EXPECT_TRUE(before != lexical_cast<std::string>(r));
        {
            cereal::BinaryInputArchive iarchive(ss);
            iarchive(r);
        }
        EXPECT_TRUE(before == lexical_cast<std::string>(r));
        EXPECT_TRUE(r.is<fair_replace>());
    }

    std::cout << r_policy{} << '\n';
}

TEST(r_policy_test, optional_tests)
{
    // get_name().
    struct udrp_00 {
        individuals_group_t replace(const individuals_group_t &inds, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double &, const individuals_group_t &) const
        {
            return inds;
        }
        std::string get_name() const
        {
            return "frobniz";
        }
    };
    EXPECT_EQ(r_policy{udrp_00{}}.get_name(), "frobniz");
    struct udrp_01 {
        individuals_group_t replace(const individuals_group_t &inds, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double &, const individuals_group_t &) const
        {
            return inds;
        }
        // Missing const.
        std::string get_name()
        {
            return "frobniz";
        }
    };
    EXPECT_TRUE(r_policy{udrp_01{}}.get_name() != "frobniz");

    // get_extra_info().
    struct udrp_02 {
        individuals_group_t replace(const individuals_group_t &inds, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double &, const individuals_group_t &) const
        {
            return inds;
        }
        std::string get_extra_info() const
        {
            return "frobniz";
        }
    };
    EXPECT_EQ(r_policy{udrp_02{}}.get_extra_info(), "frobniz");
    struct udrp_03 {
        individuals_group_t replace(const individuals_group_t &inds, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double &, const individuals_group_t &) const
        {
            return inds;
        }
        // Missing const.
        std::string get_extra_info()
        {
            return "frobniz";
        }
    };
    EXPECT_TRUE(r_policy{udrp_03{}}.get_extra_info().empty());
}

TEST(r_policy_test, stream_operator)
{
    struct udrp_00 {
        individuals_group_t replace(const individuals_group_t &inds, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double &, const individuals_group_t &) const
        {
            return inds;
        }
    };
    {
        std::ostringstream oss;
        oss << r_policy{udrp_00{}};
        EXPECT_TRUE(!oss.str().empty());
    }
    struct udrp_01 {
        individuals_group_t replace(const individuals_group_t &inds, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double &, const individuals_group_t &) const
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
        oss << r_policy{udrp_01{}};
        const auto st = oss.str();
        EXPECT_TRUE(st.contains("bartoppo"));
        EXPECT_TRUE(st.contains("Extra info:"));
    }
}

TEST(r_policy_test, replace)
{
    r_policy r0;

    EXPECT_THROW(r0.replace(individuals_group_t{{0}, {}, {}}, 0, 0, 0, 0, 0, {}, individuals_group_t{}), policy_config_error);

    EXPECT_THROW(r0.replace(individuals_group_t{{0}, {{1.}}, {{1.}}}, 0, 0, 0, 0, 0, {},
                            individuals_group_t{{0}, {{1.}, {1.}}, {{1.}}}), policy_config_error);

    EXPECT_THROW(r0.replace(individuals_group_t{{0}, {{1.}}, {{1.}}}, 0, 0, 0, 0, 0, {},
                            individuals_group_t{{0}, {{1.}}, {{1.}}}), policy_config_error);

    EXPECT_THROW(r0.replace(individuals_group_t{{0}, {{1.}}, {{1.}}}, 1, 2, 0, 0, 0, {},
                            individuals_group_t{{0}, {{1.}}, {{1.}}}), policy_config_error);

    EXPECT_THROW(r0.replace(individuals_group_t{{0}, {{1.}}, {{1.}}}, 1, 0, 0, 0, 0, {},
                   individuals_group_t{{0}, {{1.}}, {{1.}}}), policy_config_error);

    EXPECT_THROW(r0.replace(individuals_group_t{{0}, {{1.}}, {{1.}}}, 1, 0, std::numeric_limits<vector_double::size_type>::max(),
                   0, 0, {}, individuals_group_t{{0}, {{1.}}, {{1.}}}), policy_config_error);

    EXPECT_THROW(r0.replace(individuals_group_t{{0}, {{1.}}, {{1.}}}, 1, 0, 1,
                            std::numeric_limits<vector_double::size_type>::max(), 0, {},
                            individuals_group_t{{0}, {{1.}}, {{1.}}}), policy_config_error);

    EXPECT_THROW(r0.replace(individuals_group_t{{0}, {{1.}}, {{1.}}}, 1, 0, 1, 0,
                            std::numeric_limits<vector_double::size_type>::max(), {},
                            individuals_group_t{{0}, {{1.}}, {{1.}}}), policy_config_error);

    EXPECT_THROW(r0.replace(individuals_group_t{{0}, {{1.}}, {{1.}}}, 1, 0, 1, 1, 1, {},
                            individuals_group_t{{0}, {{1.}}, {{1.}}}), policy_config_error);

    EXPECT_THROW(r0.replace(individuals_group_t{{0, 1}, {{1.}, {}}, {{1.}, {1.}}}, 1, 0, 1, 0, 0, {},
                            individuals_group_t{{0}, {{1.}}, {{1.}}}), policy_config_error);

    EXPECT_THROW(r0.replace(individuals_group_t{{0, 1}, {{1.}, {1.}}, {{1.}, {}}}, 1, 0, 1, 0, 0, {},
                            individuals_group_t{{0}, {{1.}}, {{1.}}}), policy_config_error);

    EXPECT_THROW(r0.replace(individuals_group_t{{0, 1}, {{1.}, {1.}}, {{1.}, {1.}}}, 1, 0, 1, 0, 0, {},
                            individuals_group_t{{0, 1}, {{1.}, {}}, {{1.}, {1.}}}), policy_config_error);

    EXPECT_THROW(r0.replace(individuals_group_t{{0, 1}, {{1.}, {1.}}, {{1.}, {1.}}}, 1, 0, 1, 0, 0, {},
                            individuals_group_t{{0, 1}, {{1.}, {1.}}, {{1.}, {}}}), policy_config_error);

    struct fail_0 {
        individuals_group_t replace(const individuals_group_t &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double &, const individuals_group_t &) const
        {
            return individuals_group_t{{0}, {}, {}};
        }
        std::string get_name() const
        {
            return "fail_0";
        }
    };

    EXPECT_THROW(r_policy{fail_0{}}.replace(individuals_group_t{{0, 1}, {{1.}, {1.}}, {{1.}, {1.}}}, 1, 0, 1, 0, 0, {},
                                            individuals_group_t{{0, 1}, {{1.}, {1.}}, {{1.}, {1.}}}), policy_config_error);

    struct fail_1 {
        individuals_group_t replace(const individuals_group_t &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double &, const individuals_group_t &) const
        {
            return individuals_group_t{{0, 1}, {{1}, {}}, {{1}, {1}}};
        }
        std::string get_name() const
        {
            return "fail_1";
        }
    };

    EXPECT_THROW(r_policy{fail_1{}}.replace(individuals_group_t{{0, 1}, {{1.}, {1.}}, {{1.}, {1.}}}, 1, 0, 1, 0, 0, {},
                                            individuals_group_t{{0, 1}, {{1.}, {1.}}, {{1.}, {1.}}}), policy_config_error);

    struct fail_2 {
        individuals_group_t replace(const individuals_group_t &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double::size_type &, const vector_double::size_type &,
                                    const vector_double &, const individuals_group_t &) const
        {
            return individuals_group_t{{0, 1}, {{1}, {1}}, {{1}, {}}};
        }
        std::string get_name() const
        {
            return "fail_2";
        }
    };

    EXPECT_THROW(r_policy{fail_2{}}.replace(individuals_group_t{{0, 1}, {{1.}, {1.}}, {{1.}, {1.}}}, 1, 0, 1, 0, 0, {},
                                            individuals_group_t{{0, 1}, {{1.}, {1.}}, {{1.}, {1.}}}), policy_config_error);
}

struct udrp_a {
    individuals_group_t replace(const individuals_group_t &inds, const vector_double::size_type &,
                                const vector_double::size_type &, const vector_double::size_type &,
                                const vector_double::size_type &, const vector_double::size_type &,
                                const vector_double &, const individuals_group_t &) const
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

PAGMO_S11N_R_POLICY_EXPORT(udrp_a)

// Serialization tests.
TEST(r_policy_test, s11n)
{
    r_policy r_pol0{udrp_a{}};
    EXPECT_TRUE(r_pol0.extract<udrp_a>()->state == 42);
    r_pol0.extract<udrp_a>()->state = -42;
    // Store the string representation.
    std::stringstream ss;
    auto before = lexical_cast<std::string>(r_pol0);
    // Now serialize, deserialize and compare the result.
    {
        cereal::BinaryOutputArchive oarchive(ss);
        oarchive(r_pol0);
    }
    // Change the content of p before deserializing.
    r_pol0 = r_policy{};
    {
        cereal::BinaryInputArchive iarchive(ss);
        iarchive(r_pol0);
    }
    auto after = lexical_cast<std::string>(r_pol0);
    EXPECT_EQ(before, after);
    EXPECT_TRUE(r_pol0.is<udrp_a>());
    EXPECT_TRUE(r_pol0.extract<udrp_a>()->state = -42);
}

TEST(r_policy_test, is_valid)
{
    r_policy p0;
    EXPECT_TRUE(p0.is_valid());
    r_policy p1(std::move(p0));
    EXPECT_TRUE(!p0.is_valid());
    p0 = r_policy{udrp_a{}};
    EXPECT_TRUE(p0.is_valid());
    p1 = std::move(p0);
    EXPECT_TRUE(!p0.is_valid());
    p0 = r_policy{udrp_a{}};
    EXPECT_TRUE(p0.is_valid());
}

TEST(r_policy_test, generic_assignment)
{
    r_policy p0;
    EXPECT_TRUE(p0.is<fair_replace>());
    EXPECT_TRUE(&(p0 = udrp_a{}) == &p0);
    EXPECT_TRUE(p0.is_valid());
    EXPECT_TRUE(p0.is<udrp_a>());
    p0 = udrp1{};
    EXPECT_TRUE(p0.is<udrp1>());
    EXPECT_TRUE((!std::is_assignable<r_policy, void>::value));
    EXPECT_TRUE((!std::is_assignable<r_policy, int &>::value));
    EXPECT_TRUE((!std::is_assignable<r_policy, const int &>::value));
    EXPECT_TRUE((!std::is_assignable<r_policy, int &&>::value));
}

TEST(r_policy_test, type_index)
{
    r_policy p0;
    EXPECT_TRUE(p0.get_type_index() == std::type_index(typeid(fair_replace)));
    p0 = r_policy{udrp1{}};
    EXPECT_TRUE(p0.get_type_index() == std::type_index(typeid(udrp1)));
}

TEST(r_policy_test, get_ptr)
{
    r_policy p0;
    EXPECT_TRUE(p0.get_ptr() == p0.extract<fair_replace>());
    EXPECT_TRUE(static_cast<const r_policy &>(p0).get_ptr()
                == static_cast<const r_policy &>(p0).extract<fair_replace>());
    p0 = r_policy{udrp1{}};
    EXPECT_TRUE(p0.get_ptr() == p0.extract<udrp1>());
    EXPECT_TRUE(static_cast<const r_policy &>(p0).get_ptr() == static_cast<const r_policy &>(p0).extract<udrp1>());
}

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

#include <gtest/gtest.h>

#include <initializer_list>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <variant>

#include <pagmo/exceptions.hpp>
#include <pagmo/r_policies/fair_replace.hpp>
#include <pagmo/r_policy.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/types.hpp>

using namespace pagmo;

TEST(fair_replace_test, fair_replace_basic)
{
    fair_replace f00;
    EXPECT_TRUE(f00.get_migr_rate().index() == 0);
    EXPECT_TRUE(std::get<pop_size_t>(f00.get_migr_rate()) == 1u);

    fair_replace f01(.2);
    EXPECT_TRUE(f01.get_migr_rate().index() == 1);
    EXPECT_TRUE(std::get<double>(f01.get_migr_rate()) == .2);

    fair_replace f02(2);
    EXPECT_TRUE(f02.get_migr_rate().index() == 0);
    EXPECT_TRUE(std::get<pop_size_t>(f02.get_migr_rate()) == 2u);

    EXPECT_THROW(f02 = fair_replace(-1.), policy_config_error);
    EXPECT_THROW(f02 = fair_replace(2.), policy_config_error);
    EXPECT_THROW(f02 = fair_replace(std::numeric_limits<double>::infinity()), policy_config_error);
    EXPECT_THROW(f02 = fair_replace(-1), std::overflow_error);

    auto f03(f02);
    EXPECT_TRUE(f03.get_migr_rate().index() == 0);
    EXPECT_TRUE(std::get<pop_size_t>(f03.get_migr_rate()) == 2u);

    auto f04(std::move(f01));
    EXPECT_TRUE(f04.get_migr_rate().index() == 1);
    EXPECT_TRUE(std::get<double>(f04.get_migr_rate()) == .2);

    f03 = f04;
    EXPECT_TRUE(f03.get_migr_rate().index() == 1);
    EXPECT_TRUE(std::get<double>(f03.get_migr_rate()) == .2);

    f04 = std::move(f02);
    EXPECT_TRUE(f04.get_migr_rate().index() == 0);
    EXPECT_TRUE(std::get<pop_size_t>(f04.get_migr_rate()) == 2u);

    EXPECT_TRUE(f04.get_name() == "Fair replace");
    EXPECT_TRUE(f04.get_extra_info().contains("Absolute migration rate:"));
    EXPECT_TRUE(f03.get_extra_info().contains("Fractional migration rate:"));

    // Minimal serialization test.
    {
        r_policy r0(f04);

        std::stringstream ss;
        {
            cereal::BinaryOutputArchive oarchive(ss);
            oarchive(r0);
        }
        r_policy r1;
        {
            cereal::BinaryInputArchive iarchive(ss);
            iarchive(r1);
        }
        EXPECT_TRUE(r1.is<fair_replace>());
        EXPECT_TRUE(r1.extract<fair_replace>()->get_migr_rate().index() == 0);
        EXPECT_TRUE(std::get<pop_size_t>(r1.extract<fair_replace>()->get_migr_rate()) == 2u);
    }
}

TEST(fair_replace_test, fair_replace_replace)
{
    fair_replace f00;

    EXPECT_THROW(f00.replace(individuals_group_t{}, 0, 0, 2, 1, 0, vector_double{}, individuals_group_t{}),
                 policy_config_error);

    f00 = fair_replace(100);

    EXPECT_THROW(f00.replace(individuals_group_t{}, 0, 0, 1, 0, 0, vector_double{}, individuals_group_t{}),
                 std::invalid_argument);

    // Single-objective, unconstrained.
    f00 = fair_replace(.1);

    individuals_group_t inds{{1, 2, 3}, {{0}, {0}, {0}}, {{1}, {2}, {3}}};
    individuals_group_t mig{{4, 5, 6}, {{0}, {0}, {0}}, {{0.1}, {0.2}, {0.3}}};

    // Too few individuals in inds for fractional migration, no migration will happen.
    auto new_inds = f00.replace(inds, 1, 0, 1, 0, 0, {}, mig);
    EXPECT_TRUE(new_inds == inds);

    // Top mig replaces the worst in inds.
    f00 = fair_replace(0.5);
    new_inds = f00.replace(inds, 1, 0, 1, 0, 0, {}, mig);
    EXPECT_TRUE((new_inds == individuals_group_t{{4, 1, 2}, {{0}, {0}, {0}}, {{0.1}, {1}, {2}}}));

    // All replaced.
    f00 = fair_replace(1.);
    new_inds = f00.replace(inds, 1, 0, 1, 0, 0, {}, mig);
    EXPECT_TRUE((new_inds == individuals_group_t{{4, 5, 6}, {{0}, {0}, {0}}, {{0.1}, {0.2}, {0.3}}}));

    // Absolute rate, no replacement.
    f00 = fair_replace(0);
    new_inds = f00.replace(inds, 1, 0, 1, 0, 0, {}, mig);
    EXPECT_TRUE(new_inds == inds);

    // Absolute rate, replace 2.
    f00 = fair_replace(2);
    new_inds = f00.replace(inds, 1, 0, 1, 0, 0, {}, mig);
    EXPECT_TRUE((new_inds == individuals_group_t{{4, 5, 1}, {{0}, {0}, {0}}, {{0.1}, {0.2}, {1}}}));

    // Absolute rate, replace all.
    f00 = fair_replace(3);
    new_inds = f00.replace(inds, 1, 0, 1, 0, 0, {}, mig);
    EXPECT_TRUE((new_inds == individuals_group_t{{4, 5, 6}, {{0}, {0}, {0}}, {{0.1}, {0.2}, {0.3}}}));

    // Single-objective, constrained.
    inds = individuals_group_t{{1, 2, 3}, {{0}, {0}, {0}}, {{1, 1, 1}, {2, 2, 2}, {3, 3, 3}}};
    mig = individuals_group_t{{4, 5, 6}, {{0}, {0}, {0}}, {{0.1, 0.1, 0.1}, {0.2, 0.2, 0.2}, {0.3, 0.3, 0.3}}};
    f00 = fair_replace(.1);

    // Too few individuals in inds for fractional migration, no migration will happen.
    new_inds = f00.replace(inds, 1, 0, 1, 1, 1, {0., 0.}, mig);
    EXPECT_TRUE(new_inds == inds);

    // Top mig replaces the worst in inds.
    f00 = fair_replace(0.5);
    new_inds = f00.replace(inds, 1, 0, 1, 1, 1, {0., 0.}, mig);
    EXPECT_TRUE((new_inds == individuals_group_t{{4, 1, 2}, {{0}, {0}, {0}}, {{0.1, 0.1, 0.1}, {1, 1, 1}, {2, 2, 2}}}));

    // All replaced.
    f00 = fair_replace(1.);
    new_inds = f00.replace(inds, 1, 0, 1, 1, 1, {0., 0.}, mig);
    EXPECT_TRUE(
        (new_inds
         == individuals_group_t{{4, 5, 6}, {{0}, {0}, {0}}, {{0.1, 0.1, 0.1}, {0.2, 0.2, 0.2}, {0.3, 0.3, 0.3}}}));

    // Absolute rate, no replacement.
    f00 = fair_replace(0);
    new_inds = f00.replace(inds, 1, 0, 1, 1, 1, {0., 0.}, mig);
    EXPECT_TRUE(new_inds == inds);

    // Absolute rate, replace 2.
    f00 = fair_replace(2);
    new_inds = f00.replace(inds, 1, 0, 1, 1, 1, {0., 0.}, mig);
    EXPECT_TRUE(
        (new_inds == individuals_group_t{{4, 5, 1}, {{0}, {0}, {0}}, {{0.1, 0.1, 0.1}, {0.2, 0.2, 0.2}, {1, 1, 1}}}));

    // Absolute rate, replace all.
    f00 = fair_replace(3);
    new_inds = f00.replace(inds, 1, 0, 1, 1, 1, {0., 0.}, mig);
    EXPECT_TRUE(
        (new_inds
         == individuals_group_t{{4, 5, 6}, {{0}, {0}, {0}}, {{0.1, 0.1, 0.1}, {0.2, 0.2, 0.2}, {0.3, 0.3, 0.3}}}));

    // Multi-objective, unconstrained.
    // NOTE: these values are taken from a test in multi_objective.cpp.
    inds = individuals_group_t{{1, 2, 3, 4, 5}, {{0}, {0}, {0}, {0}, {0}}, {{0, 7}, {1, 5}, {2, 3}, {4, 2}, {7, 1}}};
    mig = individuals_group_t{
        {6, 7, 8, 9, 10, 11}, {{0}, {0}, {0}, {0}, {0}, {0}}, {{10, 0}, {2, 6}, {4, 4}, {10, 2}, {6, 6}, {9, 5}}};
    f00 = fair_replace(1.);

    new_inds = f00.replace(inds, 1, 0, 2, 0, 0, {}, mig);
    EXPECT_TRUE((
        new_inds
        == individuals_group_t{{1, 6, 5, 4, 2}, {{0}, {0}, {0}, {0}, {0}}, {{0, 7}, {10, 0}, {7, 1}, {4, 2}, {1, 5}}}));
}

// Check behaviour when, in unconstrained
// single-objective optimisation problems,
// the fitness is nan.
TEST(fair_replace_test, fair_replace_nan_fitness)
{
    fair_replace f00(1);

    individuals_group_t inds{{1, 2, 3}, {{0}, {0}, {0}}, {{1}, {2}, {3}}};
    individuals_group_t mig{
        {4, 5}, {{0}, {0}}, {{std::numeric_limits<double>::quiet_NaN()}, {std::numeric_limits<double>::quiet_NaN()}}};

    auto new_inds = f00.replace(inds, 1, 0, 1, 0, 0, {}, mig);

    // No individual was replaced, because the migrant has nan fitness.
    EXPECT_TRUE(inds == new_inds);
}

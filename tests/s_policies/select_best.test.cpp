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
#include <pagmo/s11n.hpp>
#include <pagmo/s_policies/select_best.hpp>
#include <pagmo/s_policy.hpp>
#include <pagmo/types.hpp>

using namespace pagmo;

TEST(select_best_test, select_best_basic)
{
    select_best f00;
    EXPECT_TRUE(f00.get_migr_rate().index() == 0);
    EXPECT_TRUE(std::get<pop_size_t>(f00.get_migr_rate()) == 1u);

    select_best f01(.2);
    EXPECT_TRUE(f01.get_migr_rate().index() == 1);
    EXPECT_TRUE(std::get<double>(f01.get_migr_rate()) == .2);

    select_best f02(2);
    EXPECT_TRUE(f02.get_migr_rate().index() == 0);
    EXPECT_TRUE(std::get<pop_size_t>(f02.get_migr_rate()) == 2u);

    EXPECT_THROW(f02 = select_best(-1.), policy_config_error);
    EXPECT_THROW(f02 = select_best(2.), policy_config_error);
    EXPECT_THROW(f02 = select_best(std::numeric_limits<double>::infinity()), policy_config_error);
    EXPECT_THROW(f02 = select_best(-1), std::overflow_error);

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

    EXPECT_TRUE(f04.get_name() == "Select best");
    EXPECT_TRUE(f04.get_extra_info().contains("Absolute migration rate:"));
    EXPECT_TRUE(f03.get_extra_info().contains("Fractional migration rate:"));

    // Minimal serialization test.
    {
        s_policy r0(f04);

        std::stringstream ss;
        {
            cereal::BinaryOutputArchive oarchive(ss);
            oarchive(r0);
        }
        s_policy r1;
        {
            cereal::BinaryInputArchive iarchive(ss);
            iarchive(r1);
        }
        EXPECT_TRUE(r1.is<select_best>());
        EXPECT_TRUE(r1.extract<select_best>()->get_migr_rate().index() == 0);
        EXPECT_TRUE(std::get<pop_size_t>(r1.extract<select_best>()->get_migr_rate()) == 2u);
    }
}

TEST(select_best_test, select_best_select)
{
    select_best f00;

    EXPECT_THROW(f00.select(individuals_group_t{}, 0, 0, 2, 1, 0, vector_double{}), policy_config_error);

    f00 = select_best(100);

    EXPECT_THROW(f00.select(individuals_group_t{}, 0, 0, 1, 0, 0, vector_double{}), std::invalid_argument);

    // Single-objective, unconstrained.
    f00 = select_best(.1);

    individuals_group_t inds{{1, 2, 3}, {{0}, {0}, {0}}, {{1}, {2}, {3}}};

    // Too few individuals in inds for fractional migration, no migration will happen.
    auto new_inds = f00.select(inds, 1, 0, 1, 0, 0, {});
    EXPECT_TRUE(new_inds == individuals_group_t{});

    // Select top 1.
    f00 = select_best(0.5);
    new_inds = f00.select(inds, 1, 0, 1, 0, 0, {});
    EXPECT_TRUE((new_inds == individuals_group_t{{1}, {{0}}, {{1}}}));

    // All selected.
    f00 = select_best(1.);
    new_inds = f00.select(inds, 1, 0, 1, 0, 0, {});
    EXPECT_TRUE((new_inds == inds));

    // Absolute rate, no selection.
    f00 = select_best(0);
    new_inds = f00.select(inds, 1, 0, 1, 0, 0, {});
    EXPECT_TRUE(new_inds == individuals_group_t{});

    // Absolute rate, select 2.
    f00 = select_best(2);
    new_inds = f00.select(inds, 1, 0, 1, 0, 0, {});
    EXPECT_TRUE((new_inds == individuals_group_t{{1, 2}, {{0}, {0}}, {{1}, {2}}}));

    // Absolute rate, select all.
    f00 = select_best(3);
    new_inds = f00.select(inds, 1, 0, 1, 0, 0, {});
    EXPECT_TRUE((new_inds == inds));

    // Single-objective, constrained.
    inds = individuals_group_t{{1, 2, 3}, {{0}, {0}, {0}}, {{1, 1, 1}, {2, 2, 2}, {3, 3, 3}}};
    f00 = select_best(.1);

    // Too few individuals in inds for fractional migration, no migration will happen.
    new_inds = f00.select(inds, 1, 0, 1, 1, 1, {0., 0.});
    EXPECT_TRUE(new_inds == individuals_group_t{});

    // Select top 1.
    f00 = select_best(0.5);
    new_inds = f00.select(inds, 1, 0, 1, 1, 1, {0., 0.});
    EXPECT_TRUE((new_inds == individuals_group_t{{1}, {{0}}, {{1, 1, 1}}}));

    // All selected.
    f00 = select_best(1.);
    new_inds = f00.select(inds, 1, 0, 1, 1, 1, {0., 0.});
    EXPECT_TRUE((new_inds == inds));

    // Absolute rate, no selection.
    f00 = select_best(0);
    new_inds = f00.select(inds, 1, 0, 1, 1, 1, {0., 0.});
    EXPECT_TRUE(new_inds == individuals_group_t{});

    // Absolute rate, select 2.
    f00 = select_best(2);
    new_inds = f00.select(inds, 1, 0, 1, 1, 1, {0., 0.});
    EXPECT_TRUE((new_inds == individuals_group_t{{1, 2}, {{0}, {0}}, {{1, 1, 1}, {2, 2, 2}}}));

    // Absolute rate, select all.
    f00 = select_best(3);
    new_inds = f00.select(inds, 1, 0, 1, 1, 1, {0., 0.});
    EXPECT_TRUE((new_inds == inds));

    // Multi-objective, unconstrained.
    // NOTE: these values are taken from a test in multi_objective.cpp.
    inds = individuals_group_t{{1, 2, 3, 4, 5}, {{0}, {0}, {0}, {0}, {0}}, {{0, 7}, {1, 5}, {2, 3}, {4, 2}, {7, 1}}};
    f00 = select_best(1.);

    new_inds = f00.select(inds, 1, 0, 2, 0, 0, {});
    EXPECT_TRUE((new_inds == inds));
}

// Check behaviour when, in unconstrained
// single-objective optimisation problems,
// the fitness is nan.
TEST(select_best_test, select_best_nan_fitness)
{
    select_best f00(2);

    individuals_group_t inds{{1, 2, 3}, {{0}, {0}, {0}}, {{1}, {2}, {std::numeric_limits<double>::quiet_NaN()}}};

    auto new_inds = f00.select(inds, 1, 0, 1, 0, 0, {});

    // Check that the individual with nan fitness was not selected.
    EXPECT_TRUE((new_inds == individuals_group_t{{1, 2}, {{0}, {0}}, {{1}, {2}}}));
}

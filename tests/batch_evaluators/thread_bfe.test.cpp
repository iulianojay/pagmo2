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
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <pagmo/batch_evaluators/thread_bfe.hpp>
#include <pagmo/bfe.hpp>
#include <pagmo/exceptions.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/problems/inventory.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/threading.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>
#include <pagmo/utils/generic.hpp>

using namespace pagmo;

static std::mt19937 rng;

TEST(thread_bfe_test, basic_tests)
{
    EXPECT_TRUE(IsUdBfe<thread_bfe>);

    bfe bfe0{thread_bfe{}};
    EXPECT_TRUE(bfe0.get_name() == "Multi-threaded batch fitness evaluator");
    EXPECT_TRUE(bfe0.get_extra_info().empty());
    EXPECT_EQ(bfe0.get_thread_safety(), thread_safety::basic);

    // Try with a problem providing the constant thread safety level.
    problem p0{rosenbrock{2}};
    // Rosenbrock has dimension 2, thus these are 5000 dvs.
    vector_double dvs(10000u);
    for (auto &x : dvs) {
        x = uniform_real_from_range(-1., 1., rng);
    }
    auto fvs = bfe0(p0, dvs);
    EXPECT_EQ(p0.get_fevals(), 5000u);
    vector_double tmp_dv(2u);
    for (decltype(dvs.size()) i = 0; i < dvs.size(); i += 2u) {
        tmp_dv[0] = dvs[i];
        tmp_dv[1] = dvs[i + 1u];
        EXPECT_TRUE(fvs[i / 2u] == p0.fitness(tmp_dv)[0]);
    }

    // Try with a problem providing the basic thread safety level.
    p0 = problem{inventory{4}};
    for (auto &x : dvs) {
        x = uniform_real_from_range(0., 1., rng);
    }
    fvs = bfe0(p0, dvs);
    EXPECT_EQ(p0.get_fevals(), 2500u);
    tmp_dv.resize(4u);
    for (decltype(dvs.size()) i = 0; i < dvs.size(); i += 4u) {
        tmp_dv[0] = dvs[i];
        tmp_dv[1] = dvs[i + 1u];
        tmp_dv[2] = dvs[i + 2u];
        tmp_dv[3] = dvs[i + 3u];
        EXPECT_TRUE(fvs[i / 4u] == p0.fitness(tmp_dv)[0]);
    }

    // A problem not providing any thread safety.
    struct unsafe_prob {
        vector_double fitness(const vector_double &) const
        {
            return vector_double{1.};
        }
        std::pair<vector_double, vector_double> get_bounds() const
        {
            return {{0.}, {1.}};
        }
        thread_safety get_thread_safety() const
        {
            return thread_safety::none;
        }
        std::string get_name() const
        {
            return "unsafe_prob";
        }
    };
    p0 = problem{unsafe_prob{}};
    EXPECT_EQ(p0.get_thread_safety(), thread_safety::none);
    EXPECT_THROW(bfe0(p0, dvs), batch_eval_error);
}

TEST(thread_bfe_test, s11n)
{
    bfe bfe0{thread_bfe{}};
    EXPECT_TRUE(bfe0.is<thread_bfe>());
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
    EXPECT_TRUE(!bfe0.is<thread_bfe>());
    {
        cereal::BinaryInputArchive iarchive(ss);
        iarchive(bfe0);
    }
    auto after = lexical_cast<std::string>(bfe0);
    EXPECT_EQ(before, after);
    EXPECT_TRUE(bfe0.is<thread_bfe>());
}
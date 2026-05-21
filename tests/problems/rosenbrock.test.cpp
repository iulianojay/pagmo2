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

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

#include <pagmo/problem.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/threading.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>
#include <pagmo/exceptions.hpp>

using namespace pagmo;

TEST(rosenbrock_test, rosenbrock_test)
{
    // Problem construction
    rosenbrock ros2{2u};
    rosenbrock ros5{5u};
    EXPECT_THROW(rosenbrock{0u}, problem_config_error);
    EXPECT_THROW(rosenbrock{1u}, problem_config_error);
    EXPECT_NO_THROW(problem{rosenbrock{2u}});
    // Pick a few reference points
    vector_double x2 = {1., 1.};
    vector_double x5 = {1., 1., 1., 1., 1.};
    // Fitness test
    EXPECT_TRUE((ros2.fitness({1., 1.}) == vector_double{0.}));
    EXPECT_TRUE((ros5.fitness({1., 1., 1., 1., 1.}) == vector_double{0.}));
    // Bounds Test
    EXPECT_TRUE((ros2.get_bounds() == std::pair<vector_double, vector_double>{{-5., -5.}, {10., 10.}}));
    // Name and extra info tests
    EXPECT_TRUE(ros5.get_name().find("Rosenbrock") != std::string::npos);
    // Best known test
    auto x_best = ros2.best_known();
    EXPECT_TRUE((x_best == vector_double{1., 1.}));
    // Gradient test.
    auto g2 = ros2.gradient({.1, .2});
    EXPECT_TRUE(std::abs(g2[0] + 9.4) < 1E-8);
    EXPECT_TRUE(std::abs(g2[1] - 38.) < 1E-8);
    auto g5 = ros5.gradient({.1, .2, .3, .4, .5});
    EXPECT_TRUE(std::abs(g5[0] + 9.4) < 1E-8);
    EXPECT_TRUE(std::abs(g5[1] - 15.6) < 1E-8);
    EXPECT_TRUE(std::abs(g5[2] - 13.4) < 1E-8);
    EXPECT_TRUE(std::abs(g5[3] - 6.4) < 1E-8);
    EXPECT_TRUE(std::abs(g5[4] - 68.) < 1E-8);
    // Thread safety level.
    EXPECT_TRUE(problem{rosenbrock{}}.get_thread_safety() == thread_safety::constant);
}

TEST(rosenbrock_test, rosenbrock_serialization_test)
{
    problem p{rosenbrock{4u}};
    // Call objfun to increase the internal counters.
    p.fitness({1., 1., 1., 1.});
    // Store the string representation of p.
    std::stringstream ss;
    auto before = lexical_cast<std::string>(p);
    // Now serialize, deserialize and compare the result.
    {
        cereal::BinaryOutputArchive oarchive(ss);
        oarchive(p);
    }
    // Change the content of p before deserializing.
    p = problem{};
    {
        cereal::BinaryInputArchive iarchive(ss);
        iarchive(p);
    }
    auto after = lexical_cast<std::string>(p);
    EXPECT_EQ(before, after);
}

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

#include <iostream>
#include <stdexcept>
#include <string>

#include <pagmo/problem.hpp>
#include <pagmo/problems/ackley.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>
#include <pagmo/exceptions.hpp>
using namespace pagmo;

TEST(ackley_test, ackley_test)
{
    // Problem construction
    ackley ack1{1u};
    ackley ack3{3u};
    EXPECT_THROW(ackley{0u}, problem_config_error);
    EXPECT_NO_THROW(problem{ack3});
    // Pick a few reference points
    vector_double x1 = {1.12};
    vector_double x3 = {-23.45, 12.34, 111.12};
    // Fitness test
    EXPECT_NEAR(ack1.fitness(x1)[0], 4.659037611351948, 1e-13);
    EXPECT_NEAR(ack1.fitness(x3)[0], 21.941495638130885, 1e-13);
    // Bounds Test
    EXPECT_TRUE((ack3.get_bounds() == std::pair<vector_double, vector_double>{{-15, -15, -15}, {30, 30, 30}}));
    // Name and extra info tests
    EXPECT_TRUE(ack3.get_name().find("Ackley") != std::string::npos);
    // Best known test
    auto x_best = ack3.best_known();
    EXPECT_TRUE((x_best == vector_double{0., 0., 0.}));
}

TEST(ackley_test, ackley_serialization_test)
{
    problem p{ackley{4u}};
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

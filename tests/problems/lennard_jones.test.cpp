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
#include <limits>
#include <pagmo/utils/cast.hpp>
#include <stdexcept>
#include <string>

#include <pagmo/problem.hpp>
#include <pagmo/problems/lennard_jones.hpp>
#include <pagmo/types.hpp>
#include <pagmo/exceptions.hpp>

using namespace pagmo;

TEST(lennard_jones_test, lennard_jones_test)
{
    // Problem construction
    EXPECT_THROW(lennard_jones{0u}, problem_config_error);
    EXPECT_THROW(lennard_jones{1u}, problem_config_error);
    EXPECT_THROW(lennard_jones{2u}, problem_config_error);
    EXPECT_THROW(lennard_jones{std::numeric_limits<unsigned>::max() / 2}, size_limit_error);

    lennard_jones lj{3u};
    EXPECT_NO_THROW(problem{lj});
    // Pick a few reference points
    vector_double x1 = {1.12, -0.33, 2.34};
    vector_double x2 = {1.23, -1.23, 0.33};
    // Fitness test
    EXPECT_NEAR(lj.fitness(x1)[0], -1.7633355813175688, 1e-13);
    EXPECT_NEAR(lj.fitness(x2)[0], -1.833100934753864, 1e-13);
    // Bounds Test
    EXPECT_TRUE((lj.get_bounds() == std::pair<vector_double, vector_double>{{-3, -3, -3}, {3, 3, 3}}));
    // Name and extra info tests
    EXPECT_TRUE(lj.get_name().find("Jones") != std::string::npos);
}

TEST(lennard_jones_test, lennard_jones_serialization_test)
{
    problem p{lennard_jones{30u}};
    // Call objfun to increase the internal counters.
    p.fitness(vector_double(30u * 3 - 6u, 0.1));
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

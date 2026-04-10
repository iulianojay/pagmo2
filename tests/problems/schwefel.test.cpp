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
#include <pagmo/utils/cast.hpp>
#include <stdexcept>
#include <string>

#include <pagmo/problem.hpp>
#include <pagmo/problems/schwefel.hpp>
#include <pagmo/types.hpp>
#include <pagmo/exceptions.hpp>

using namespace pagmo;

TEST(schwefel_test, schwefel_test)
{
    // Problem construction
    schwefel sch1{1u};
    schwefel sch3{3u};
    EXPECT_THROW(schwefel{0u}, problem_config_error);
    EXPECT_NO_THROW(problem{sch3});
    // Pick a few reference points
    vector_double x1 = {1.12};
    vector_double x3 = {-23.45, 12.34, 111.12};
    // Fitness test
    EXPECT_NEAR(sch1.fitness(x1)[0], 418.0067810680098, 1e-13);
    EXPECT_NEAR(sch1.fitness(x3)[0], 1338.0260195323838, 1e-13);
    // Bounds Test
    EXPECT_TRUE((sch3.get_bounds() == std::pair<vector_double, vector_double>{{-500, -500, -500}, {500, 500, 500}}));
    // Name and extra info tests
    EXPECT_TRUE(sch3.get_name().find("Schwefel") != std::string::npos);
    // Best known test
    auto x_best = sch3.best_known();
    EXPECT_TRUE((x_best == vector_double{420.9687, 420.9687, 420.9687}));
}

TEST(schwefel_test, schwefel_serialization_test)
{
    problem p{schwefel{4u}};
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

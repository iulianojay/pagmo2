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
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <pagmo/problem.hpp>
#include <pagmo/problems/cec2013.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>
#include <pagmo/utils/generic.hpp>
#include <pagmo/exceptions.hpp>

using namespace pagmo;

TEST(cec2013_test, cec2013_test)
{
    std::mt19937 r_engine(32u);
    // We check that all problems can be constructed at all dimensions and that the name returned makes sense
    // (only for dim =2 for speed). We also perform a fitness test (we only check no throws, not correctness)
    std::vector<unsigned> allowed_dims = {2u, 5u, 10u, 20u, 30u, 40u, 50u, 60u, 70u, 80u, 90u, 100u};
    for (unsigned i = 1u; i <= 28u; ++i) {
        for (auto dim : allowed_dims) {
            cec2013 udp{i, dim};
            auto x = random_decision_vector(problem(udp), r_engine); // a random vector
            EXPECT_NO_THROW(udp.fitness(x));
        }
        EXPECT_TRUE((cec2013{i, 2u}.get_name().find("CEC2013 - f")) != std::string::npos);
    }
    // We check that wrong problem ids and dimensions cannot be constructed
    EXPECT_THROW((cec2013{0u, 2u}), problem_config_error);
    EXPECT_THROW((cec2013{29u, 2u}), problem_config_error);
    EXPECT_THROW((cec2013{10u, 3u}), problem_config_error);
}

TEST(cec2013_test, cec2013_serialization_test)
{
    problem p{cec2013{1u, 2u}};
    // Call objfun to increase the internal counters.
    p.fitness(vector_double(2u, 0.));
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

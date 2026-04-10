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

#include <pagmo/detail/constants.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/problems/rastrigin.hpp>
#include <pagmo/types.hpp>
#include <pagmo/exceptions.hpp>

using namespace pagmo;

TEST(rastrigin_test, rastrigin_test)
{
    // Problem construction
    rastrigin ras1{1u};
    rastrigin ras5{5u};
    EXPECT_THROW(rastrigin{0u}, problem_config_error);
    EXPECT_NO_THROW(problem{rastrigin{2u}});
    // Pick a few reference points
    vector_double x1 = {1.};
    vector_double x5 = {1., 1., 1., 1., 1.};
    // Fitness test
    EXPECT_TRUE((ras1.fitness(x1) == vector_double{1.}));
    EXPECT_TRUE((ras5.fitness(x5) == vector_double{5.}));
    // Gradient test
    auto g1 = ras1.gradient(x1);
    auto g5 = ras5.gradient(x5);
    for (decltype(g1.size()) i = 0u; i < g1.size(); ++i) {
        EXPECT_NEAR(g1[i], 2., 1e-12);
    }
    for (decltype(g5.size()) i = 0u; i < g5.size(); ++i) {
        EXPECT_NEAR(g5[i], 2., 1e-12);
    }
    // Hessians test
    auto h1 = ras1.hessians(x1);
    auto h5 = ras5.hessians(x5);
    for (decltype(h1[0].size()) i = 0u; i < h1[0].size(); ++i) {
        EXPECT_NEAR(h1[0][i], 2. + 4 * detail::pi() * detail::pi() * 10, 1e-12);
    }
    for (decltype(h5[0].size()) i = 0u; i < h5[0].size(); ++i) {
        EXPECT_NEAR(h5[0][i], 2. + 4 * detail::pi() * detail::pi() * 10, 1e-12);
    }
    // Hessian sparsity test
    EXPECT_TRUE((ras1.hessians_sparsity() == std::vector<sparsity_pattern>{{{0, 0}}}));
    EXPECT_TRUE((ras5.hessians_sparsity() == std::vector<sparsity_pattern>{{{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}}}));
    // Bounds Test
    EXPECT_TRUE((ras1.get_bounds() == std::pair<vector_double, vector_double>{{-5.12}, {5.12}}));
    // Name and extra info tests
    EXPECT_TRUE(ras5.get_name().find("Rastrigin") != std::string::npos);
    // Best known test
    auto x_best = ras5.best_known();
    EXPECT_TRUE((x_best == vector_double{0., 0., 0., 0., 0.}));
}

TEST(rastrigin_test, rastrigin_serialization_test)
{
    problem p{rastrigin{4u}};
    // Call objfun to increase the internal counters.
    p.fitness({1., 1., 1., 1.});
    p.gradient({1., 1., 1., 1.});
    p.hessians({1., 1., 1., 1.});
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

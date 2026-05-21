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
#include <string>

#include <pagmo/problems/hock_schittkowski_71.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>

using namespace pagmo;

TEST(hock_schittkowski_test, hock_schittkowski_71_test)
{
    // Problem instantiation
    problem p{hock_schittkowski_71{}};
    // Pick a few reference points
    vector_double x1 = {1., 1., 1., 1.};
    vector_double x2 = {2., 2., 2., 2.};
    // Fitness test
    EXPECT_TRUE((p.fitness(x1) == vector_double{4, -36, 24}));
    EXPECT_TRUE((p.fitness(x2) == vector_double{26, -24, 9}));
    // Gradient test
    EXPECT_TRUE((p.gradient(x1) == vector_double{4, 1, 2, 3, 2, 2, 2, 2, -1, -1, -1, -1}));
    EXPECT_TRUE((p.gradient(x2) == vector_double{16, 4, 5, 12, 4, 4, 4, 4, -8, -8, -8, -8}));
    // Hessians test
    auto hess1 = p.hessians(x1);
    EXPECT_TRUE(hess1.size() == 3);
    EXPECT_TRUE((hess1[0] == vector_double{2, 1, 1, 4, 1, 1}));
    EXPECT_TRUE((hess1[1] == vector_double{2, 2, 2, 2}));
    EXPECT_TRUE((hess1[2] == vector_double{-1, -1, -1, -1, -1, -1}));
    // Hessians sparsity test
    auto sp = p.hessians_sparsity();
    EXPECT_TRUE(sp.size() == 3);
    EXPECT_TRUE((sp[0] == sparsity_pattern{{0, 0}, {1, 0}, {2, 0}, {3, 0}, {3, 1}, {3, 2}}));
    EXPECT_TRUE((sp[1] == sparsity_pattern{{0, 0}, {1, 1}, {2, 2}, {3, 3}}));
    EXPECT_TRUE((sp[2] == sparsity_pattern{{1, 0}, {2, 0}, {2, 1}, {3, 0}, {3, 1}, {3, 2}}));
    // Name and extra info tests
    EXPECT_TRUE(p.get_name().find("Schittkowski") != std::string::npos);
    EXPECT_TRUE(p.get_extra_info().find("Schittkowski") != std::string::npos);
    // Best known test
    auto x_best = p.extract<hock_schittkowski_71>()->best_known();
    EXPECT_NEAR(x_best[0], 1, 1e-13);
    EXPECT_NEAR(x_best[1], 4.74299963, 1e-13);
    EXPECT_NEAR(x_best[2], 3.82114998, 1e-13);
    EXPECT_NEAR(x_best[3], 1.37940829, 1e-13);
}

TEST(hock_schittkowski_test, hock_schittkowski_71_serialization_test)
{
    problem p{hock_schittkowski_71{}};
    // Call objfun, grad and hess to increase
    // the internal counters.
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

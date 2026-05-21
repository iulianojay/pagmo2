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

#include <stdexcept>

#include <pagmo/exceptions.hpp>
#include <pagmo/io.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/constrained.hpp>

using namespace pagmo;

TEST(constrained_test, compare_fc_test)
{
    vector_double f1 = {2., 1.};
    vector_double f2 = {1., 2.};
    vector_double f3 = {1., -2.};
    vector_double f4 = {1., -3., -5.};
    vector_double f5 = {0.2, 1., 2.};
    vector_double tol = {0., 0.};
    vector_double empty = {};
    EXPECT_TRUE(compare_fc(f1, f2, 1u, 0.) == true);
    EXPECT_TRUE(compare_fc(f2, f1, 1u, 0.) == false);
    EXPECT_TRUE(compare_fc(f1, f3, 0u, 0.) == false);
    EXPECT_TRUE(compare_fc(f4, f5, 2u, 0.) == false);
    EXPECT_TRUE(compare_fc(f4, f5, 1u, 0.) == true);
    EXPECT_TRUE(compare_fc(f4, f5, 2u, tol) == false);
    EXPECT_TRUE(compare_fc(f4, f5, 1u, tol) == true);

    EXPECT_THROW(compare_fc(f1, f5, 1u, 0.), dimension_mismatch_error);
    EXPECT_THROW(compare_fc(f1, f2, 3u, 0.), bounds_constraint_error);
    EXPECT_THROW(compare_fc(f1, f2, 1u, tol), dimension_mismatch_error);
    EXPECT_THROW(compare_fc(empty, empty, 1u, 0.), multi_objective_error);
    EXPECT_THROW(compare_fc(empty, empty, 1u, tol), multi_objective_error);
}

TEST(constrained_test, sort_population_con_test)
{
    std::vector<vector_double> example;
    vector_double::size_type neq;
    vector_double tol;
    std::vector<vector_double::size_type> result;
    // Test 1 - check on known cases
    example = {{0, 0, 0}, {1, 1, 0}, {2, 0, 0}};
    neq = 1;
    result = {0, 2, 1};
    tol = {0., 0.};
    EXPECT_TRUE(sort_population_con(example, neq) == result);
    EXPECT_TRUE(sort_population_con(example, neq, 0.) == result);
    EXPECT_TRUE(sort_population_con(example, neq, tol) == result);
    example = {{0, 0, 0}, {1, 0, 0}, {2, 0, 0}};
    neq = 1;
    result = {0, 1, 2};
    tol = {0., 0.};
    EXPECT_TRUE(sort_population_con(example, neq) == result);
    EXPECT_TRUE(sort_population_con(example, neq, 0.) == result);
    EXPECT_TRUE(sort_population_con(example, neq, tol) == result);
    example = {{-1, 0, -20}, {0, 0, -1}, {1, 0, -2}};
    neq = 1;
    result = {0, 1, 2};
    tol = {0., 0.};
    EXPECT_TRUE(sort_population_con(example, neq) == result);
    EXPECT_TRUE(sort_population_con(example, neq, 0.) == result);
    EXPECT_TRUE(sort_population_con(example, neq, tol) == result);
    example = {{-1, 0, -20}, {0, 0, -1}, {1, 0, -2}};
    neq = 2;
    result = {1, 2, 0};
    tol = {0., 0.};
    EXPECT_TRUE(sort_population_con(example, neq) == result);
    EXPECT_TRUE(sort_population_con(example, neq, 0.) == result);
    EXPECT_TRUE(sort_population_con(example, neq, tol) == result);
    example = {{-1, 0, 0}, {0, 0, -1}, {1, 0, 0}};
    neq = 2;
    result = {0, 1, 2};
    tol = {0., 1.};
    EXPECT_TRUE(sort_population_con(example, neq) != result);
    EXPECT_TRUE(sort_population_con(example, neq, 0.) != result);
    EXPECT_TRUE(sort_population_con(example, neq, tol) == result);
    example = {{-1, 0, -20}, {0, 0, -1}, {1, 0, -2}};
    neq = 0;
    result = {0, 1, 2};
    tol = {0., 0.};
    EXPECT_TRUE(sort_population_con(example, neq) == result);
    EXPECT_TRUE(sort_population_con(example, neq, 0.) == result);
    EXPECT_TRUE(sort_population_con(example, neq, tol) == result);
    example = {{1}, {0}, {2}, {3}};
    neq = 0;
    result = {1, 0, 2, 3};
    EXPECT_TRUE(sort_population_con(example, neq) == result);
    // Test corner cases
    example = {};
    neq = 0;
    result = {};
    tol = {2., 3., 4.};
    EXPECT_TRUE(sort_population_con(example, neq, tol) == result);
    EXPECT_TRUE(sort_population_con(example, neq) == result);
    example = {{1}};
    neq = 0;
    result = {0};
    EXPECT_TRUE(sort_population_con(example, neq) == result);
    EXPECT_TRUE(sort_population_con(example, neq, tol) == result);
    // Test throws
    example = {{1, 2, 3}, {1, 2}};
    EXPECT_THROW(sort_population_con(example, neq), dimension_mismatch_error);
    example = {{-1, 0, 0}, {0, 0, -1}, {1, 0, 0}};
    EXPECT_THROW(sort_population_con(example, 3), bounds_constraint_error);
    EXPECT_THROW(sort_population_con(example, 4), bounds_constraint_error);
    tol = {2, 3, 4};
    EXPECT_THROW(sort_population_con(example, 0, tol), dimension_mismatch_error);
    tol = {2};
    EXPECT_THROW(sort_population_con(example, 0, tol), dimension_mismatch_error);
    example = {{}, {}};
    EXPECT_THROW(sort_population_con(example, 0), multi_objective_error);
    EXPECT_THROW(sort_population_con(example, 0, tol), multi_objective_error);
}

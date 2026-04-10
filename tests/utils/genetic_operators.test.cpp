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

#include <limits>

#include <pagmo/exceptions.hpp>
#include <pagmo/utils/genetic_operators.hpp>

using namespace pagmo;

TEST(generic_test, sbx_crossover_test)
{
    detail::random_engine_type random_engine(32u);
    auto nan = std::numeric_limits<double>::quiet_NaN();
    auto inf = std::numeric_limits<double>::infinity();
    EXPECT_NO_THROW(
        sbx_crossover({0.1, 0.2, 3}, {0.2, 2.2, -1}, {{-2, -2, -2}, {3, 3, 3}}, 1u, 0.9, 10, random_engine));
    EXPECT_THROW(sbx_crossover({0.1, 0.2}, {0.2, 2.2, -1}, {{-2, -2, -2}, {3, 3, 3}}, 1u, 0.9, 10, random_engine),
                 dimension_mismatch_error);
    EXPECT_THROW(sbx_crossover({0.1, 0.2, 0.3}, {0.2, 2.2, -1}, {{}, {}}, 1u, 0.9, 10, random_engine),
                 bounds_constraint_error);
    EXPECT_THROW(sbx_crossover({0.1, 0.2, 0.3}, {0.2, 2.2, -1}, {{-2, -2}, {3, 3, 3}}, 1u, 0.9, 10, random_engine),
                 dimension_mismatch_error);
    EXPECT_THROW(sbx_crossover({0.1, 0.2, 0.3}, {0.2, 2.2, -1}, {{-2, -2}, {3, 3}}, 1u, 0.9, 10, random_engine),
                 dimension_mismatch_error);
    EXPECT_THROW(sbx_crossover({0.1, 0.2, 0.3}, {0.2, 2.2, -1}, {{nan, -2, -2}, {3, 3, 3}}, 1u, 0.9, 10, random_engine),
                 invalid_value_error);
    EXPECT_THROW(
        sbx_crossover({0.1, 0.2, 0.3}, {0.2, 2.2, -1}, {{-2, -2, -2}, {3, inf, 3}}, 1u, 0.9, 10, random_engine),
        invalid_value_error);
    EXPECT_THROW(
        sbx_crossover({0.1, 0.2, 0.3}, {0.2, 2.2, -1}, {{-2, -2, -2}, {3, nan, 3}}, 1u, 0.9, 10, random_engine),
        invalid_value_error);
    EXPECT_THROW(
        sbx_crossover({0.1, 0.2, 0.3}, {0.2, 2.2, -1}, {{-2, -2, -inf}, {3, 3, 3}}, 1u, 0.9, 10, random_engine),
        invalid_value_error);
    EXPECT_THROW(sbx_crossover({0.1, 0.2, 0.3}, {0.2, 2.2, -1}, {{-2, -2, -2}, {3, -5, 3}}, 1u, 0.9, 10, random_engine),
                 invalid_value_error);
    EXPECT_THROW(sbx_crossover({0.1, 0.2, 0.3}, {0.2, 2.2, -1}, {{-2, 8, -2}, {3, 3, 3}}, 1u, 0.9, 10, random_engine),
                 invalid_value_error);
    EXPECT_THROW(sbx_crossover({0.1, 0.2, 0.3}, {0.2, 2.2, -1}, {{-2, -2, -2}, {3, 3, 3}}, 32u, 0.9, 10, random_engine),
                 bounds_constraint_error);
    EXPECT_THROW(
        sbx_crossover({0.1, 0.2, 0.3}, {0.2, 2.2, -1}, {{-2, -2, -2.6}, {3, 3, 3}}, 1u, 0.9, 10, random_engine),
        invalid_value_error);
    EXPECT_THROW(
        sbx_crossover({0.1, 0.2, 0.3}, {0.2, 2.2, -1}, {{-2, -2, -2}, {3, 3, 3.43}}, 1u, 0.9, 10, random_engine),
        invalid_value_error);
    EXPECT_THROW(sbx_crossover({0.1, 0.2, 0.3}, {0.2, 2.2, -1}, {{-2, -2, -2}, {3, 3, 3}}, 1u, nan, 10, random_engine),
                 invalid_value_error);
    EXPECT_THROW(sbx_crossover({0.1, 0.2, 0.3}, {0.2, 2.2, -1}, {{-2, -2, -2}, {3, 3, 3}}, 1u, 0.4, nan, random_engine),
                 invalid_value_error);
}

TEST(generic_test, polynomial_mutation_test)
{
    detail::random_engine_type random_engine(32u);
    auto nan = std::numeric_limits<double>::quiet_NaN();
    auto inf = std::numeric_limits<double>::infinity();
    vector_double dv = {-0.3, 2.4, 5};
    EXPECT_NO_THROW(polynomial_mutation(dv, {{-2, -2, -2}, {3, 3, 3}}, 1u, 0.9, 10, random_engine));
    EXPECT_THROW(polynomial_mutation(dv, {{}, {}}, 1u, 0.9, 10, random_engine), bounds_constraint_error);
    EXPECT_THROW(polynomial_mutation(dv, {{-2, -2}, {3, 3, 3}}, 1u, 0.9, 10, random_engine), dimension_mismatch_error);
    EXPECT_THROW(polynomial_mutation(dv, {{-2, -2}, {3, 3}}, 1u, 0.9, 10, random_engine), dimension_mismatch_error);
    EXPECT_THROW(polynomial_mutation(dv, {{nan, -2, -2}, {3, 3, 3}}, 1u, 0.9, 10, random_engine), invalid_value_error);
    EXPECT_THROW(polynomial_mutation(dv, {{-2, -2, -2}, {3, inf, 3}}, 1u, 0.9, 10, random_engine), invalid_value_error);
    EXPECT_THROW(polynomial_mutation(dv, {{-2, -2, -2}, {3, nan, 3}}, 1u, 0.9, 10, random_engine), invalid_value_error);
    EXPECT_THROW(polynomial_mutation(dv, {{-2, -2, -inf}, {3, 3, 3}}, 1u, 0.9, 10, random_engine), invalid_value_error);
    EXPECT_THROW(polynomial_mutation(dv, {{-2, -2, -2}, {3, -5, 3}}, 1u, 0.9, 10, random_engine), invalid_value_error);
    EXPECT_THROW(polynomial_mutation(dv, {{-2, 8, -2}, {3, 3, 3}}, 1u, 0.9, 10, random_engine), invalid_value_error);
    EXPECT_THROW(polynomial_mutation(dv, {{-2, -2, -2}, {3, 3, 3}}, 32u, 0.9, 10, random_engine),
                 bounds_constraint_error);
    EXPECT_THROW(polynomial_mutation(dv, {{-2, -2, -2.6}, {3, 3, 3}}, 1u, 0.9, 10, random_engine), invalid_value_error);
    EXPECT_THROW(polynomial_mutation(dv, {{-2, -2, -2}, {3, 3, 3.43}}, 1u, 0.9, 10, random_engine),
                 invalid_value_error);
    EXPECT_THROW(polynomial_mutation(dv, {{-2, -2, -2}, {3, 3, 3}}, 1u, nan, 10, random_engine), invalid_value_error);
    EXPECT_THROW(polynomial_mutation(dv, {{-2, -2, -2}, {3, 3, 3}}, 1u, 0.4, nan, random_engine), invalid_value_error);
}
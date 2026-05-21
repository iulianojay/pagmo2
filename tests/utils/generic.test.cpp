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
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <utility>

#include <pagmo/exceptions.hpp>
#include <pagmo/io.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/rng.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/generic.hpp>

using namespace pagmo;

// A UDP with user-defined bounds.
struct udp00 {
    udp00() = default;
    explicit udp00(vector_double lb, vector_double ub, vector_double::size_type nix = 0)
        : m_lb(lb), m_ub(ub), m_nix(nix)
    {
    }
    vector_double fitness(const vector_double &) const
    {
        return {0};
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {m_lb, m_ub};
    }
    vector_double::size_type get_nix() const
    {
        return m_nix;
    }
    vector_double m_lb, m_ub;
    vector_double::size_type m_nix;
};

TEST(generic_test, uniform_real_from_range_test)
{
    auto inf = std::numeric_limits<double>::infinity();
    auto big = std::numeric_limits<double>::max();
    auto nan = std::numeric_limits<double>::quiet_NaN();
    detail::random_engine_type r_engine(pagmo::random_device::next());

    // Test the throws
    EXPECT_THROW(uniform_real_from_range(1, 0, r_engine), utility_error);
    EXPECT_THROW(uniform_real_from_range(-big, big, r_engine), utility_error);
    EXPECT_THROW(uniform_real_from_range(-3, inf, r_engine), utility_error);
    EXPECT_THROW(uniform_real_from_range(-nan, nan, r_engine), utility_error);
    EXPECT_THROW(uniform_real_from_range(nan, nan, r_engine), utility_error);
    EXPECT_THROW(uniform_real_from_range(-nan, 3, r_engine), utility_error);
    EXPECT_THROW(uniform_real_from_range(-3, nan, r_engine), utility_error);
    EXPECT_THROW(uniform_real_from_range(inf, inf, r_engine), utility_error);

    EXPECT_THROW(uniform_integral_from_range(1, 0, r_engine), utility_error);
    EXPECT_THROW(uniform_integral_from_range(0, inf, r_engine), utility_error);
    EXPECT_THROW(uniform_integral_from_range(-inf, 0, r_engine), utility_error);
    EXPECT_THROW(uniform_integral_from_range(-inf, inf, r_engine), utility_error);
    EXPECT_THROW(uniform_integral_from_range(0, nan, r_engine), utility_error);
    EXPECT_THROW(uniform_integral_from_range(-nan, 0, r_engine), utility_error);
    EXPECT_THROW(uniform_integral_from_range(-nan, nan, r_engine), utility_error);
    EXPECT_THROW(uniform_integral_from_range(0, .1, r_engine), utility_error);
    EXPECT_THROW(uniform_integral_from_range(0.1, 2, r_engine), utility_error);
    EXPECT_THROW(uniform_integral_from_range(0.1, 0.2, r_engine), utility_error);
    if (big > static_cast<double>(std::numeric_limits<long long>::max())
        && -big < static_cast<double>(std::numeric_limits<long long>::min())) {
        EXPECT_THROW(uniform_integral_from_range(0, big, r_engine), utility_error);
        EXPECT_THROW(uniform_integral_from_range(-big, 0, r_engine), utility_error);
    }
}

TEST(generic_test, random_decision_vector_test)
{
    auto inf = std::numeric_limits<double>::infinity();
    auto big = std::numeric_limits<double>::max();
    detail::random_engine_type r_engine(pagmo::random_device::next());

    // Test the throws
    EXPECT_THROW(random_decision_vector(problem{udp00{{0}, {inf}}}, r_engine), utility_error);
    EXPECT_THROW(random_decision_vector(problem{udp00{{-inf}, {0}}}, r_engine), utility_error);
    EXPECT_THROW(random_decision_vector(problem{udp00{{-inf}, {inf}}}, r_engine), utility_error);
    EXPECT_THROW(random_decision_vector(problem{udp00{{-big}, {big}}}, r_engine), utility_error);
    EXPECT_THROW(random_decision_vector(problem{udp00{{0, 0}, {1, inf}, 1}}, r_engine), utility_error);
    EXPECT_THROW(random_decision_vector(problem{udp00{{0, -inf}, {1, 0}, 1}}, r_engine), utility_error);
    EXPECT_THROW(random_decision_vector(problem{udp00{{0, -inf}, {1, inf}, 1}}, r_engine), utility_error);
    if (big > static_cast<double>(std::numeric_limits<long long>::max())
        && -big < static_cast<double>(std::numeric_limits<long long>::min())) {
        EXPECT_THROW(random_decision_vector(problem{udp00{{0, 0}, {1, big}, 1}}, r_engine), utility_error);
        EXPECT_THROW(random_decision_vector(problem{udp00{{0, -big}, {1, 0}, 1}}, r_engine), utility_error);
    }

    // Test the results
    EXPECT_TRUE((random_decision_vector(problem{udp00{{3, 4}, {3, 4}}}, r_engine) == vector_double{3, 4}));
    EXPECT_TRUE((random_decision_vector(problem{udp00{{3, 4}, {3, 4}, 1}}, r_engine) == vector_double{3, 4}));
    EXPECT_TRUE((random_decision_vector(problem{udp00{{0, 0}, {1, 1}}}, r_engine)[0] >= 0.));
    EXPECT_TRUE((random_decision_vector(problem{udp00{{0, 0}, {1, 1}}}, r_engine)[1] < 1.));
    EXPECT_TRUE((random_decision_vector(problem{udp00{{0, 0}, {1, 0}}}, r_engine)[1] == 0.));
    for (auto i = 0; i < 100; ++i) {
        const auto tmp = random_decision_vector(problem{udp00{{0}, {2}, 1}}, r_engine)[0];
        EXPECT_TRUE(tmp == 0. || tmp == 1. || tmp == 2.);
    }
    for (auto i = 0; i < 100; ++i) {
        const auto res = random_decision_vector(problem{udp00{{0, -20}, {1, 20}, 1}}, r_engine);
        EXPECT_TRUE(std::trunc(res[1]) == res[1]);
    }
}

TEST(generic_test, batch_random_decision_vector_test)
{
    auto inf = std::numeric_limits<double>::infinity();
    auto big = std::numeric_limits<double>::max();
    detail::random_engine_type r_engine(pagmo::random_device::next());

    // Test the throws
    EXPECT_THROW(batch_random_decision_vector(problem{udp00{{0}, {inf}}}, 0, r_engine), utility_error);
    EXPECT_THROW(batch_random_decision_vector(problem{udp00{{-inf}, {0}}}, 0, r_engine), utility_error);
    EXPECT_THROW(batch_random_decision_vector(problem{udp00{{-inf}, {inf}}}, 0, r_engine), utility_error);
    EXPECT_THROW(batch_random_decision_vector(problem{udp00{{-big}, {big}}}, 0, r_engine), utility_error);
    EXPECT_THROW(batch_random_decision_vector(problem{udp00{{0, 0}, {1, inf}, 1}}, 0, r_engine), utility_error);
    EXPECT_THROW(batch_random_decision_vector(problem{udp00{{0, -inf}, {1, 0}, 1}}, 0, r_engine), utility_error);
    EXPECT_THROW(batch_random_decision_vector(problem{udp00{{0, -inf}, {1, inf}, 1}}, 0, r_engine), utility_error);
    if (big > static_cast<double>(std::numeric_limits<long long>::max())
        && -big < static_cast<double>(std::numeric_limits<long long>::min())) {
        EXPECT_THROW(batch_random_decision_vector(problem{udp00{{0, 0}, {1, big}, 1}}, 10, r_engine), utility_error);
        EXPECT_THROW(batch_random_decision_vector(problem{udp00{{0, -big}, {1, 0}, 1}}, 10, r_engine), utility_error);
    }

    // Test the results
    EXPECT_TRUE(batch_random_decision_vector(problem{udp00{{3, 4}, {3, 4}}}, 0, r_engine).empty());
    EXPECT_TRUE(
        (batch_random_decision_vector(problem{udp00{{3, 4}, {3, 4}}}, 3, r_engine) == vector_double{3, 4, 3, 4, 3, 4}));
    EXPECT_TRUE((batch_random_decision_vector(problem{udp00{{3, 4}, {3, 4}, 1}}, 3, r_engine)
                 == vector_double{3, 4, 3, 4, 3, 4}));
    auto tmp = batch_random_decision_vector(problem{udp00{{0, 0}, {1, 1}}}, 3, r_engine);
    EXPECT_TRUE(tmp.size() == 6u);
    EXPECT_TRUE(tmp[0] >= 0.);
    EXPECT_TRUE(tmp[2] >= 0.);
    EXPECT_TRUE(tmp[4] >= 0.);
    EXPECT_TRUE(tmp[1] < 1.);
    EXPECT_TRUE(tmp[3] < 1.);
    EXPECT_TRUE(tmp[5] < 1.);
    tmp = batch_random_decision_vector(problem{udp00{{0, 0}, {1, 0}}}, 3, r_engine);
    EXPECT_TRUE(tmp.size() == 6u);
    EXPECT_TRUE(tmp[1] == 0.);
    EXPECT_TRUE(tmp[3] == 0.);
    EXPECT_TRUE(tmp[5] == 0.);
    for (auto i = 0; i < 100; ++i) {
        tmp = batch_random_decision_vector(problem{udp00{{0}, {2}, 1}}, 3, r_engine);
        EXPECT_TRUE(tmp.size() == 3u);
        EXPECT_TRUE(tmp[0] == 0. || tmp[0] == 1. || tmp[0] == 2.);
        EXPECT_TRUE(tmp[1] == 0. || tmp[1] == 1. || tmp[1] == 2.);
        EXPECT_TRUE(tmp[2] == 0. || tmp[2] == 1. || tmp[2] == 2.);
    }
    for (auto i = 0; i < 100; ++i) {
        tmp = batch_random_decision_vector(problem{udp00{{0, -20}, {1, 20}, 1}}, 3, r_engine);
        EXPECT_TRUE(tmp.size() == 6u);
        EXPECT_TRUE(std::trunc(tmp[1]) == tmp[1]);
        EXPECT_TRUE(std::trunc(tmp[3]) == tmp[3]);
        EXPECT_TRUE(std::trunc(tmp[5]) == tmp[5]);
    }
}

TEST(generic_test, force_bounds_test)
{
    detail::random_engine_type r_engine(32u);
    // force_bounds_random
    {
        vector_double x{1., 2., 3.};
        vector_double x_fix = x;
        detail::force_bounds_random(x_fix, {0., 0., 0.}, {3., 3., 3.}, r_engine);
        EXPECT_TRUE(x == x_fix);
        detail::force_bounds_random(x_fix, {0., 0., 0.}, {1., 1., 1.}, r_engine);
        EXPECT_TRUE(x != x_fix);
        EXPECT_EQ(x_fix[0], 1.);
        EXPECT_TRUE(x_fix[1] <= 1. && x_fix[1] >= 0.);
        EXPECT_TRUE(x_fix[2] <= 1. && x_fix[2] >= 0.);
    }
    // force_bounds_reflection
    {
        vector_double x{1., 2., 5.};
        vector_double x_fix = x;
        detail::force_bounds_reflection(x_fix, {0., 0., 0.}, {3., 3., 5.});
        EXPECT_TRUE(x == x_fix);
        detail::force_bounds_reflection(x_fix, {0., 0., 0.}, {1., 1.9, 2.1});
        EXPECT_TRUE(x != x_fix);
        EXPECT_EQ(x_fix[0], 1.);
        EXPECT_NEAR(x_fix[1], 1.8, 1e-8);
        EXPECT_NEAR(x_fix[2], 0.8, 1e-8);
    }
    // force_bounds_stick
    {
        vector_double x{1., 2., 5.};
        vector_double x_fix = x;
        detail::force_bounds_stick(x_fix, {0., 0., 0.}, {3., 3., 5.});
        EXPECT_TRUE(x == x_fix);
        // ub
        detail::force_bounds_stick(x_fix, {0., 0., 0.}, {1., 1.9, 2.1});
        EXPECT_TRUE(x != x_fix);
        EXPECT_EQ(x_fix[0], 1.);
        EXPECT_EQ(x_fix[1], 1.9);
        EXPECT_EQ(x_fix[2], 2.1);
        // lb
        detail::force_bounds_stick(x_fix, {2., 2., 2.}, {3., 3., 3.});
        EXPECT_EQ(x_fix[0], 2.);
        EXPECT_EQ(x_fix[1], 2.);
        EXPECT_EQ(x_fix[2], 2.1);
    }
}

TEST(generic_test, binomial_coefficient_test)
{
    EXPECT_EQ(binomial_coefficient(0u, 0u), 1u);
    EXPECT_EQ(binomial_coefficient(1u, 0u), 1u);
    EXPECT_EQ(binomial_coefficient(1u, 1u), 1u);
    EXPECT_EQ(binomial_coefficient(2u, 0u), 1u);
    EXPECT_EQ(binomial_coefficient(2u, 1u), 2u);
    EXPECT_EQ(binomial_coefficient(2u, 2u), 1u);
    EXPECT_EQ(binomial_coefficient(13u, 5u), 1287u);
    EXPECT_EQ(binomial_coefficient(21u, 10u), 352716u);
    EXPECT_THROW(binomial_coefficient(10u, 21u), utility_error);
    EXPECT_THROW(binomial_coefficient(0u, 1u), utility_error);
    EXPECT_THROW(binomial_coefficient(4u, 7u), utility_error);
}

TEST(generic_test, kNN_test)
{
    // Corner cases
    {
        std::vector<vector_double> points = {};
        std::vector<std::vector<vector_double::size_type>> res = {};
        EXPECT_TRUE(kNN(points, 2u) == res);
    }
    {
        std::vector<vector_double> points = {{1.2, 1.2}};
        std::vector<std::vector<vector_double::size_type>> res = {{}};
        EXPECT_TRUE(kNN(points, 2u) == res);
    }
    {
        std::vector<vector_double> points = {{1.2, 1.2}, {1.2, 1.2}};
        std::vector<std::vector<vector_double::size_type>> res = {{1u}, {0u}};
        EXPECT_TRUE(kNN(points, 2u) == res);
    }
    {
        std::vector<vector_double> points = {{1, 1}, {2, 2}, {3.1, 3.1}};
        std::vector<std::vector<vector_double::size_type>> res = {{1u, 2u}, {0u, 2u}, {1u, 0u}};
        EXPECT_TRUE(kNN(points, 2u) == res);
    }

    // Some test cases
    {
        std::vector<vector_double> points = {{1, 1}, {2, 2}, {3.1, 3.1}, {4.2, 4.2}, {5.4, 5.4}};
        std::vector<std::vector<vector_double::size_type>> res
            = {{1u, 2u, 3u}, {0u, 2u, 3u}, {1u, 3u, 0u}, {2u, 4u, 1u}, {3u, 2u, 1u}};
        EXPECT_TRUE(kNN(points, 3u) == res);
    }
    // throws
    {
        std::vector<vector_double> points = {{1, 1}, {2, 2}, {2, 3, 4}};
        EXPECT_THROW(kNN(points, 3u), dimension_mismatch_error);
    }
}

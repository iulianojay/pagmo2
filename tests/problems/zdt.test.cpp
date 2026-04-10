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
#include <pagmo/problems/zdt.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>
#include <pagmo/exceptions.hpp>

using namespace pagmo;

TEST(zdt_test, zdt_construction_test)
{
    zdt zdt_default{};
    zdt zdt5{5, 11};

    EXPECT_THROW((zdt{7, 23}), problem_config_error);
    EXPECT_THROW((zdt{2, 1}), problem_config_error);
    EXPECT_NO_THROW(problem{zdt_default});
    EXPECT_NO_THROW(problem{zdt5});
    // We also test get_nobj() here as not to add one more small test
    EXPECT_TRUE(zdt_default.get_nobj() == 2u);
    // We also test get_name()
    EXPECT_TRUE(zdt5.get_name().find("ZDT5") != std::string::npos);
    // And the integer dimension
    EXPECT_TRUE(problem(zdt5).get_nix() == (11u - 1u) * 5u + 30u);
    EXPECT_TRUE(problem(zdt5).get_ncx() == 0u);
    EXPECT_TRUE(problem(zdt_default).get_nix() == 0u);
}

TEST(zdt_test, zdt1_fitness_test)
{
    {
        zdt zdt1{1, 30};
        vector_double x(30, 0.25);
        EXPECT_NEAR(zdt1.fitness(x)[0], 0.25, 1e-13);
        EXPECT_NEAR(zdt1.fitness(x)[1], 2.3486121811340026, 1e-13);
    }
    zdt zdt1{1, 13};
    {
        vector_double x(13, 0.33);
        EXPECT_NEAR(zdt1.fitness(x)[0], 0.33, 1e-13);
        EXPECT_NEAR(zdt1.fitness(x)[1], 2.825404001404863, 1e-13);
    }
}

TEST(zdt_test, zdt2_fitness_test)
{
    {
        zdt zdt2{2, 30};
        vector_double x(30, 0.25);
        EXPECT_NEAR(zdt2.fitness(x)[0], 0.25, 1e-13);
        EXPECT_NEAR(zdt2.fitness(x)[1], 3.230769230769231, 1e-13);
    }
    zdt zdt2{2, 13};
    {
        vector_double x(13, 0.33);
        EXPECT_NEAR(zdt2.fitness(x)[0], 0.33, 1e-13);
        EXPECT_NEAR(zdt2.fitness(x)[1], 3.9425692695214107, 1e-13);
    }
}

TEST(zdt_test, zdt3_fitness_test)
{
    {
        zdt zdt3{3, 30};
        vector_double x(30, 0.25);
        EXPECT_NEAR(zdt3.fitness(x)[0], 0.25, 1e-13);
        EXPECT_NEAR(zdt3.fitness(x)[1], 2.0986121811340026, 1e-13);
    }
    zdt zdt3{3, 13};
    {
        vector_double x(13, 0.33);
        EXPECT_NEAR(zdt3.fitness(x)[0], 0.33, 1e-13);
        EXPECT_NEAR(zdt3.fitness(x)[1], 3.092379609548596, 1e-13);
    }
}

TEST(zdt_test, zdt4_fitness_test)
{
    {
        zdt zdt4{4, 30};
        vector_double x(30, 0.25);
        EXPECT_NEAR(zdt4.fitness(x)[0], 0.25, 1e-13);
        EXPECT_NEAR(zdt4.fitness(x)[1], 570.7417450526075, 1e-13);
    }
    zdt zdt4{4, 13};
    {
        vector_double x(13, 0.33);
        EXPECT_NEAR(zdt4.fitness(x)[0], 0.33, 1e-13);
        EXPECT_NEAR(zdt4.fitness(x)[1], 178.75872382132619, 1e-13);
    }
}

TEST(zdt_test, zdt5_fitness_test)
{
    {
        zdt zdt5{5, 30};
        vector_double x(175, 1.);
        std::fill(x.begin() + 100, x.end(), 0.);
        EXPECT_NEAR(zdt5.fitness(x)[0], 31., 1e-13);
        EXPECT_NEAR(zdt5.fitness(x)[1], 1.4193548387096775, 1e-13);
    }
    {
        zdt zdt5{5, 13};
        vector_double x(90, 1.);
        std::fill(x.begin() + 45, x.end(), 0.);
        EXPECT_NEAR(zdt5.fitness(x)[0], 31., 1e-13);
        EXPECT_NEAR(zdt5.fitness(x)[1], 0.6774193548387096, 1e-13);
    }
    // Test with double relaxation
    {
        zdt zdt5{5, 30};
        vector_double x(175, 1.35422);
        std::fill(x.begin() + 100, x.end(), 0.1534567);
        EXPECT_NEAR(zdt5.fitness(x)[0], 31., 1e-13);
        EXPECT_NEAR(zdt5.fitness(x)[1], 1.4193548387096775, 1e-13);
    }
    {
        zdt zdt5{5, 13};
        vector_double x(90, 1.34677824);
        std::fill(x.begin() + 45, x.end(), 0.345345);
        EXPECT_NEAR(zdt5.fitness(x)[0], 31., 1e-13);
        EXPECT_NEAR(zdt5.fitness(x)[1], 0.6774193548387096, 1e-13);
    }
}

TEST(zdt_test, zdt6_fitness_test)
{
    {
        zdt zdt6{6, 30};
        vector_double x(30, 0.25);
        EXPECT_NEAR(zdt6.fitness(x)[0], 0.6321205588285577, 1e-13);
        EXPECT_NEAR(zdt6.fitness(x)[1], 7.309699961231513, 1e-13);
    }
    {
        zdt zdt6{6, 13};
        vector_double x(13, 0.33);
        EXPECT_NEAR(zdt6.fitness(x)[0], 0.999999983628226, 1e-13);
        EXPECT_NEAR(zdt6.fitness(x)[1], 7.693505388431892, 1e-13);
    }
}

TEST(zdt_test, zdt_p_distance_test)
{
    zdt zdt1{1, 30};
    zdt zdt2{2, 30};
    zdt zdt3{3, 30};
    zdt zdt4{4, 10};
    zdt zdt5{5, 11};
    zdt zdt6{6, 10};
    vector_double x(30, 0.143);
    vector_double xi(175, 1.);
    std::fill(xi.begin() + 100, xi.end(), 0.);
    EXPECT_NEAR(zdt1.p_distance(x), 1.2869999999999997, 1e-13);
    EXPECT_NEAR(zdt2.p_distance(x), 1.2869999999999997, 1e-13);
    EXPECT_NEAR(zdt3.p_distance(x), 1.2869999999999997, 1e-13);
    EXPECT_NEAR(zdt4.p_distance(x), 355.63154167532053, 1e-13);
    EXPECT_NEAR(zdt5.p_distance(xi), 15., 1e-13);
    EXPECT_NEAR(zdt6.p_distance(x), 5.534476131480399, 1e-13);
}

TEST(zdt_test, zdt_get_bounds_test)
{
    zdt zdt1{1, 30};
    zdt zdt2{2, 30};
    zdt zdt3{3, 30};
    zdt zdt4{4, 10};
    zdt zdt5{5, 11};
    zdt zdt6{6, 10};
    std::pair<vector_double, vector_double> bounds123({vector_double(30, 0.), vector_double(30, 1.)});
    std::pair<vector_double, vector_double> bounds4({vector_double(10, -5.), vector_double(10, 5.)});
    std::pair<vector_double, vector_double> bounds5({vector_double(80, 0.), vector_double(80, 1.)});
    std::pair<vector_double, vector_double> bounds6({vector_double(10, 0.), vector_double(10, 1.)});
    bounds4.first[0] = 0.;
    bounds4.second[0] = 1.;

    EXPECT_TRUE(zdt1.get_bounds() == bounds123);
    EXPECT_TRUE(zdt2.get_bounds() == bounds123);
    EXPECT_TRUE(zdt3.get_bounds() == bounds123);
    EXPECT_TRUE(zdt4.get_bounds() == bounds4);
    EXPECT_TRUE(zdt5.get_bounds() == bounds5);
    EXPECT_TRUE(zdt6.get_bounds() == bounds6);
}

TEST(zdt_test, zdt_serialization_test)
{
    problem p{zdt{4, 4}};
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

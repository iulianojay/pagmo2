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
#include <pagmo/problems/wfg.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>
#include <pagmo/exceptions.hpp>

using namespace pagmo;

TEST(wfg_test, wfg_construction_test)
{
    wfg wfg_default{};
    wfg wfg1{1u, 10u, 5u, 8u};

    EXPECT_THROW((wfg{10u, 5u, 3u, 4u}), problem_config_error);
    EXPECT_THROW((wfg{0u, 5u, 3u, 4u}), problem_config_error);
    EXPECT_THROW((wfg{8u, 0u, 3u, 4u}), problem_config_error);
    EXPECT_THROW((wfg{8u, 5u, 1u, 4u}), problem_config_error);
    EXPECT_THROW((wfg{8u, 5u, 3u, 6u}), problem_config_error);
    EXPECT_THROW((wfg{8u, 5u, 3u, 0u}), problem_config_error);
    EXPECT_THROW((wfg{8u, 5u, 4u, 4u}), problem_config_error);
    EXPECT_THROW((wfg{2u, 9u, 3u, 4u}), problem_config_error);
    EXPECT_THROW((wfg{3u, 9u, 3u, 4u}), problem_config_error);

    EXPECT_NO_THROW(problem{wfg_default});
    EXPECT_NO_THROW(problem{wfg1});
    // We also test get_nobj() here as not to add one more small test
    EXPECT_TRUE(wfg1.get_nobj() == 5u);
    EXPECT_TRUE(wfg_default.get_nobj() == 3u);
    // We also test get_name()
    EXPECT_TRUE(wfg1.get_name().find("WFG1") != std::string::npos);
    // And the decision vector dimension
    EXPECT_TRUE(problem(wfg1).get_nx() == 10u);
    EXPECT_TRUE(problem(wfg_default).get_nx() == 5u);
}

TEST(wfg_test, wfg1_fitness_test)
{
    wfg wfg1{1, 9, 5, 8};
    vector_double x(9, 2);
    EXPECT_NEAR(wfg1.fitness(x)[0], 2.67637472191165, 1e-6);
    EXPECT_NEAR(wfg1.fitness(x)[1], 1.00059019674296, 1e-6);
    EXPECT_NEAR(wfg1.fitness(x)[2], 1.00158344827345, 1e-6);
    EXPECT_NEAR(wfg1.fitness(x)[3], 0.999721693168825, 1e-6);
    EXPECT_NEAR(wfg1.fitness(x)[4], 0.994938703521363, 1e-6);
}

TEST(wfg_test, wfg2_fitness_test)
{
    wfg wfg2{2, 10, 5, 8};
    vector_double x(10, 2);
    EXPECT_NEAR(wfg2.fitness(x)[0], 0.486888085871606, 1e-6);
    EXPECT_NEAR(wfg2.fitness(x)[1], 0.495069130688985, 1e-6);
    EXPECT_NEAR(wfg2.fitness(x)[2], 0.760259287323669, 1e-6);
    EXPECT_NEAR(wfg2.fitness(x)[3], 3.2410479386539, 1e-6);
    EXPECT_NEAR(wfg2.fitness(x)[4], 6.7367724867725, 1e-6);
}

TEST(wfg_test, wfg3_fitness_test)
{
    wfg wfg3{3, 10, 5, 8};
    vector_double x(10, 2);
    EXPECT_NEAR(wfg3.fitness(x)[0], 0.553316039900195, 1e-6);
    EXPECT_NEAR(wfg3.fitness(x)[1], 0.767247897430556, 1e-6);
    EXPECT_NEAR(wfg3.fitness(x)[2], 1.66007950505305, 1e-6);
    EXPECT_NEAR(wfg3.fitness(x)[3], 4.09523809523809, 1e-6);
    EXPECT_NEAR(wfg3.fitness(x)[4], 2.98677248677249, 1e-6);
}

TEST(wfg_test, wfg4_fitness_test)
{
    wfg wfg4{4, 9, 5, 8};
    vector_double x(9, 2);
    EXPECT_NEAR(wfg4.fitness(x)[0], 0.50411484109126659, 1e-6);
    EXPECT_NEAR(wfg4.fitness(x)[1], 0.62464631796973902, 1e-6);
    EXPECT_NEAR(wfg4.fitness(x)[2], 1.52629658797989287, 1e-6);
    EXPECT_NEAR(wfg4.fitness(x)[3], 6.09678409164092994, 1e-6);
    EXPECT_NEAR(wfg4.fitness(x)[4], 7.24371673029278185, 1e-6);
}

TEST(wfg_test, wfg5_fitness_test)
{
    wfg wfg5{5, 9, 5, 8};
    vector_double x(9, 2);
    EXPECT_NEAR(wfg5.fitness(x)[0], 0.89534187984089331, 1e-6);
    EXPECT_NEAR(wfg5.fitness(x)[1], 1.77719665056195542, 1e-6);
    EXPECT_NEAR(wfg5.fitness(x)[2], 2.65432694359806565, 1e-6);
    EXPECT_NEAR(wfg5.fitness(x)[3], 1.53267330147733283, 1e-6);
    EXPECT_NEAR(wfg5.fitness(x)[4], 8.29285568212296020, 1e-6);
}

TEST(wfg_test, wfg6_fitness_test)
{
    wfg wfg6{6, 9, 5, 8};
    vector_double x(9, 2);
    EXPECT_NEAR(wfg6.fitness(x)[0], 0.70886239871081658, 1e-6);
    EXPECT_NEAR(wfg6.fitness(x)[1], 1.01095392496448455, 1e-6);
    EXPECT_NEAR(wfg6.fitness(x)[2], 2.84355886471448649, 1e-6);
    EXPECT_NEAR(wfg6.fitness(x)[3], 7.82173248919986541, 1e-6);
    EXPECT_NEAR(wfg6.fitness(x)[4], 3.27073013356489017, 1e-6);
}

TEST(wfg_test, wfg7_fitness_test)
{
    wfg wfg7{7, 9, 5, 8};
    vector_double x(9, 2);
    EXPECT_NEAR(wfg7.fitness(x)[0], 1.84130317215186778, 1e-6);
    EXPECT_NEAR(wfg7.fitness(x)[1], 2.30307723148680399, 1e-6);
    EXPECT_NEAR(wfg7.fitness(x)[2], 3.52250245655475958, 1e-6);
    EXPECT_NEAR(wfg7.fitness(x)[3], 4.61486710981617687, 1e-6);
    EXPECT_NEAR(wfg7.fitness(x)[4], 2.54081486970198434, 1e-6);
}

TEST(wfg_test, wfg8_fitness_test)
{
    wfg wfg8{8, 9, 5, 8};
    vector_double x(9, 2);
    EXPECT_NEAR(wfg8.fitness(x)[0], 0.415416373194518, 1e-6);
    EXPECT_NEAR(wfg8.fitness(x)[1], 0.820930156178991, 1e-6);
    EXPECT_NEAR(wfg8.fitness(x)[2], 2.71771179680979, 1e-6);
    EXPECT_NEAR(wfg8.fitness(x)[3], 6.99576478246945, 1e-6);
    EXPECT_NEAR(wfg8.fitness(x)[4], 4.19378015276566, 1e-6);
}

TEST(wfg_test, wfg9_fitness_test)
{
    wfg wfg9{9, 9, 5, 8};
    vector_double x(9, 2);
    EXPECT_NEAR(wfg9.fitness(x)[0], 0.70102740452657, 1e-6);
    EXPECT_NEAR(wfg9.fitness(x)[1], 1.08669966382728, 1e-6);
    EXPECT_NEAR(wfg9.fitness(x)[2], 2.03157023149974, 1e-6);
    EXPECT_NEAR(wfg9.fitness(x)[3], 4.14060740683114, 1e-6);
    EXPECT_NEAR(wfg9.fitness(x)[4], 8.71813196317622, 1e-6);
}

TEST(wfg_test, wfg_get_bounds_test)
{
    wfg wfg1{1u, 4u, 2u, 2u};
    wfg wfg2{2u, 4u, 2u, 2u};
    wfg wfg3{3u, 6u, 2u, 2u};
    wfg wfg4{4u, 6u, 2u, 2u};
    wfg wfg5{5u, 8u, 2u, 2u};
    wfg wfg6{6u, 8u, 2u, 2u};
    wfg wfg7{7u, 10u, 2u, 2u};
    wfg wfg8{8u, 10u, 2u, 2u};
    wfg wfg9{9u, 4u, 2u, 2u};
    std::pair<vector_double, vector_double> bounds129({vector_double(4, 0.), {2, 4, 6, 8}});
    std::pair<vector_double, vector_double> bounds34({vector_double(6, 0.), {2, 4, 6, 8, 10, 12}});
    std::pair<vector_double, vector_double> bounds56({vector_double(8, 0.), {2, 4, 6, 8, 10, 12, 14, 16}});
    std::pair<vector_double, vector_double> bounds78({vector_double(10, 0.), {2, 4, 6, 8, 10, 12, 14, 16, 18, 20}});

    EXPECT_TRUE(wfg1.get_bounds() == bounds129);
    EXPECT_TRUE(wfg2.get_bounds() == bounds129);
    EXPECT_TRUE(wfg3.get_bounds() == bounds34);
    EXPECT_TRUE(wfg4.get_bounds() == bounds34);
    EXPECT_TRUE(wfg5.get_bounds() == bounds56);
    EXPECT_TRUE(wfg6.get_bounds() == bounds56);
    EXPECT_TRUE(wfg7.get_bounds() == bounds78);
    EXPECT_TRUE(wfg8.get_bounds() == bounds78);
    EXPECT_TRUE(wfg9.get_bounds() == bounds129);
}

TEST(wfg_test, wfg_serialization_test)
{
    problem p{wfg{1u, 4u, 2u, 2u}};
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

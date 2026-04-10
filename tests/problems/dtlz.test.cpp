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
#include <stdexcept>
#include <string>

#include <pagmo/problem.hpp>
#include <pagmo/problems/dtlz.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>
#include <pagmo/exceptions.hpp>

using namespace pagmo;

TEST(dtlz_test, zdt_construction_test)
{
    dtlz dtlz_default{};
    dtlz dtlz5{5u, 7u, 3u, 100u};

    EXPECT_THROW((dtlz{0u, 7u, 3u, 100u}), problem_config_error);
    EXPECT_THROW((dtlz{9u, 7u, 3u, 100u}), problem_config_error);
    EXPECT_THROW((dtlz{1u, 7u, 1u, 100u}), problem_config_error);
    EXPECT_THROW((dtlz{1u, 7u, std::numeric_limits<vector_double::size_type>::max() - 1u, 100u}), problem_config_error);
    EXPECT_THROW((dtlz{1u, std::numeric_limits<vector_double::size_type>::max() - 1u, 3u, 100u}), problem_config_error);
    EXPECT_THROW((dtlz{1u, 3u, 3u, 100u}), problem_config_error);

    EXPECT_NO_THROW(problem{dtlz_default});
    EXPECT_NO_THROW(problem{dtlz5});
    // We also test get_nobj() here as not to add one more small test
    EXPECT_TRUE(dtlz_default.get_nobj() == 3u);
    // We also test get_name()
    EXPECT_TRUE(dtlz5.get_name().find("DTLZ5") != std::string::npos);

    EXPECT_TRUE(problem{dtlz5}.get_nx() == 7u);
    EXPECT_TRUE(dtlz5.get_bounds().first.size() == 7u);
}

TEST(dtlz_test, dtlz1_fitness_test)
{
    vector_double dv1{0.5, 0.5, 0.5, 0.5, 0.5};
    vector_double dv2{0.1, 0.2, 0.3, 0.4, 0.5};
    vector_double f1, f2;
    // dtlz1
    dtlz udp{1u, 5u, 3u};
    f1 = {0.125, 0.125, 0.25};
    f2 = {0.059999999999999824, 0.2399999999999993, 2.699999999999992};
    for (unsigned i = 0u; i < 3u; ++i) {
        EXPECT_NEAR(udp.fitness(dv1)[i], f1[i], 1e-12);
    }
    for (unsigned i = 0u; i < 3u; ++i) {
        EXPECT_NEAR(udp.fitness(dv2)[i], f2[i], 1e-12);
    }
}

TEST(dtlz_test, dtlz2_fitness_test)
{
    vector_double dv1{0.5, 0.5, 0.5, 0.5, 0.5};
    vector_double dv2{0.1, 0.2, 0.3, 0.4, 0.5};
    vector_double f1, f2;
    // dtlz1
    dtlz udp{2u, 5u, 3u};
    f1 = {0.5000000000000001, 0.5, 0.7071067811865475};
    f2 = {0.9863148040113404, 0.3204731065093832, 0.16425618829224242};
    for (unsigned i = 0u; i < 3u; ++i) {
        EXPECT_NEAR(udp.fitness(dv1)[i], f1[i], 1e-12);
    }
    for (unsigned i = 0u; i < 3u; ++i) {
        EXPECT_NEAR(udp.fitness(dv2)[i], f2[i], 1e-12);
    }
}

TEST(dtlz_test, dtlz3_fitness_test)
{
    vector_double dv1{0.5, 0.5, 0.5, 0.5, 0.5};
    vector_double dv2{0.1, 0.2, 0.3, 0.4, 0.5};
    vector_double f1, f2;
    // dtlz1
    dtlz udp{3u, 5u, 3u};
    f1 = {0.5000000000000001, 0.5, 0.7071067811865475};
    f2 = {5.6360845943505, 1.8312748943393273, 0.9386067902413824};
    for (unsigned i = 0u; i < 3u; ++i) {
        EXPECT_NEAR(udp.fitness(dv1)[i], f1[i], 1e-12);
    }
    for (unsigned i = 0u; i < 3u; ++i) {
        EXPECT_NEAR(udp.fitness(dv2)[i], f2[i], 1e-12);
    }
}

TEST(dtlz_test, dtlz4_fitness_test)
{
    vector_double dv1{0.5, 0.5, 0.5, 0.5, 0.5};
    vector_double dv2{0.1, 0.2, 0.3, 0.4, 0.5};
    vector_double f1, f2;
    // dtlz1
    dtlz udp{4u, 5u, 3u};
    f1 = {1.0, 1.2391398122732624e-30, 1.2391398122732624e-30};
    f2 = {1.05, 2.090781951822753e-70, 1.6493361431346507e-100};
    for (unsigned i = 0u; i < 3u; ++i) {
        EXPECT_NEAR(udp.fitness(dv1)[i], f1[i], 1e-12);
    }
    for (unsigned i = 0u; i < 3u; ++i) {
        EXPECT_NEAR(udp.fitness(dv2)[i], f2[i], 1e-12);
    }
}

TEST(dtlz_test, dtlz5_fitness_test)
{
    vector_double dv1{0.5, 0.5, 0.5, 0.5, 0.5};
    vector_double dv2{0.1, 0.2, 0.3, 0.4, 0.5};
    vector_double f1, f2;
    // dtlz1
    dtlz udp{5u, 5u, 3u};
    f1 = {0.5000000000000001, 0.5, 0.7071067811865475};
    f2 = {0.7495908626265831, 0.7166822470763723, 0.16425618829224242};
    for (unsigned i = 0u; i < 3u; ++i) {
        EXPECT_NEAR(udp.fitness(dv1)[i], f1[i], 1e-12);
    }
    for (unsigned i = 0u; i < 3u; ++i) {
        EXPECT_NEAR(udp.fitness(dv2)[i], f2[i], 1e-12);
    }
}

TEST(dtlz_test, dtlz6_fitness_test)
{
    vector_double dv1{0.5, 0.5, 0.5, 0.5, 0.5};
    vector_double dv2{0.1, 0.2, 0.3, 0.4, 0.5};
    vector_double f1, f2;
    // dtlz1
    dtlz udp{6u, 5u, 3u};
    f1 = {1.8995494873052114, 1.8995494873052112, 2.6863686473458888};
    f2 = {3.3343308165801333, 1.5714799440921394, 0.5838204128120267};
    for (unsigned i = 0u; i < 3u; ++i) {
        EXPECT_NEAR(udp.fitness(dv1)[i], f1[i], 1e-12);
    }
    for (unsigned i = 0u; i < 3u; ++i) {
        EXPECT_NEAR(udp.fitness(dv2)[i], f2[i], 1e-12);
    }
}

TEST(dtlz_test, dtlz7_fitness_test)
{
    vector_double dv1{0.5, 0.5, 0.5, 0.5, 0.5};
    vector_double dv2{0.1, 0.2, 0.3, 0.4, 0.5};
    vector_double f1, f2;
    // dtlz1
    dtlz udp{7u, 5u, 3u};
    f1 = {0.5, 0.5, 19.5};
    f2 = {0.1, 0.2, 16.228886997303473};
    for (unsigned i = 0u; i < 3u; ++i) {
        EXPECT_NEAR(udp.fitness(dv1)[i], f1[i], 1e-12);
    }
    for (unsigned i = 0u; i < 3u; ++i) {
        EXPECT_NEAR(udp.fitness(dv2)[i], f2[i], 1e-12);
    }
}

TEST(dtlz_test, dtlz_get_bounds_test)
{
    std::pair<vector_double, vector_double> bounds({vector_double(4, 0.), vector_double(4, 1.)});
    for (unsigned i = 1u; i <= 7u; ++i) {
        dtlz udp{i, 4u};
        EXPECT_TRUE(udp.get_bounds() == bounds);
    }
}

TEST(dtlz_test, dtlz_p_distance_test)
{
    vector_double x(4u, 0.231);
    vector_double x_wrong(3u, 0.231);
    // The following numbers were computed in PyGMO legacy
    vector_double res = {288.09711053693565,  0.14472200000000002, 288.09711053693565, 0.14472200000000002,
                         0.14472200000000002, 1.7273931523406256,  2.0790000000000002};
    for (unsigned i = 1u; i <= 7u; ++i) {
        dtlz udp{i, 4u};
        EXPECT_NEAR(udp.p_distance(x), res[i - 1u], 1e-12);
    }
    dtlz udp{3u, 4u};
    EXPECT_THROW(udp.p_distance(x_wrong), dimension_mismatch_error);
    EXPECT_NO_THROW(udp.p_distance(population{udp, 20u, 32u}));
}

TEST(dtlz_test, dtlz_serialization_test)
{
    problem p{dtlz{4u, 4u}};
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

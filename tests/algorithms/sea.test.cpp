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

#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/sea.hpp>
#include <pagmo/io.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problems/hock_schittkowski_71.hpp>
#include <pagmo/problems/inventory.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/problems/zdt.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>
#include <pagmo/exceptions.hpp>

using namespace pagmo;
using namespace std;

TEST(sea_test, sea_algorithm_construction)
{
    sea user_algo{1234u, 42u};
    EXPECT_TRUE(user_algo.get_verbosity() == 0u);
    EXPECT_TRUE(user_algo.get_seed() == 42u);
    EXPECT_TRUE((user_algo.get_log() == sea::log_type{}));
}

TEST(sea_test, sea_evolve_test)
{
    // Here we only test that evolution is deterministic if the
    // seed is controlled.
    //
    // On a single objective deterministic problem
    {
        problem prob{rosenbrock{25u}};
        population pop1{prob, 5u, 23u};
        sea user_algo1{10u, 23u};
        user_algo1.set_verbosity(1u);
        pop1 = user_algo1.evolve(pop1);

        population pop2{prob, 5u, 23u};
        sea user_algo2{10u, 23u};
        user_algo2.set_verbosity(1u);
        pop2 = user_algo2.evolve(pop2);

        EXPECT_TRUE(user_algo1.get_log() == user_algo2.get_log());

        population pop3{prob, 5u, 23u};
        user_algo2.set_seed(23u);
        pop3 = user_algo2.evolve(pop3);

        EXPECT_TRUE(user_algo1.get_log() == user_algo2.get_log());
    }
    // On a single objective stochastic problem
    {
        problem prob{inventory{25u, 5u, 1432u}};
        population pop1{prob, 5u, 23u};
        sea user_algo1{10u, 23u};
        user_algo1.set_verbosity(2u);
        pop1 = user_algo1.evolve(pop1);

        population pop2{prob, 5u, 23u};
        sea user_algo2{10u, 23u};
        user_algo2.set_verbosity(2u); // more verbosity here to also cover the relative code lines
        pop2 = user_algo2.evolve(pop2);

        EXPECT_TRUE(user_algo1.get_log() == user_algo2.get_log());

        population pop3{prob, 5u, 23u};
        user_algo2.set_seed(23u);
        pop3 = user_algo2.evolve(pop3);

        EXPECT_TRUE(user_algo1.get_log() == user_algo2.get_log());
    }
    // We then check that the evolve throws if called on unsuitable problems
    EXPECT_THROW(sea{10u}.evolve(population{problem{zdt{}}, 5u, 23u}), incompatible_problem_error);
    EXPECT_THROW(sea{10u}.evolve(population{problem{hock_schittkowski_71{}}, 5u, 23u}), incompatible_problem_error);
    // Or with not enough individuals
    EXPECT_THROW(sea{10u}.evolve(population{problem{rosenbrock{}}, 0u}), insufficient_population_error);
    // And a clean exit for 0 generations
    population pop{rosenbrock{25u}, 10u};
    EXPECT_TRUE(sea{0u}.evolve(pop).get_x()[0] == pop.get_x()[0]);
}

TEST(sea_test, sea_setters_getters_test)
{
    sea user_algo{10u, 23u};
    user_algo.set_verbosity(23u);
    EXPECT_TRUE(user_algo.get_verbosity() == 23u);
    user_algo.set_seed(23u);
    EXPECT_TRUE(user_algo.get_seed() == 23u);
    EXPECT_TRUE(user_algo.get_name().find("Simple Evolutionary Algorithm") != std::string::npos);
    EXPECT_TRUE(user_algo.get_extra_info().find("Verbosity") != std::string::npos);
    EXPECT_NO_THROW(user_algo.get_log());
}

TEST(sea_test, sea_serialization_test)
{
    // Make one evolution
    problem prob{rosenbrock{25u}};
    population pop{prob, 5u, 23u};
    algorithm algo{sea{10u, 23u}};
    algo.set_verbosity(1u);
    pop = algo.evolve(pop);

    // Store the string representation of p.
    std::stringstream ss;
    auto before_text = lexical_cast<std::string>(algo);
    auto before_log = algo.extract<sea>()->get_log();
    // Now serialize, deserialize and compare the result.
    {
        cereal::BinaryOutputArchive oarchive(ss);
        oarchive(algo);
    }
    // Change the content of p before deserializing.
    algo = algorithm{};
    {
        cereal::BinaryInputArchive iarchive(ss);
        iarchive(algo);
    }
    auto after_text = lexical_cast<std::string>(algo);
    auto after_log = algo.extract<sea>()->get_log();
    EXPECT_EQ(before_text, after_text);
    EXPECT_TRUE(before_log == after_log);
    // so we implement a close check
    for (auto i = 0u; i < before_log.size(); ++i) {
        EXPECT_EQ(std::get<0>(before_log[i]), std::get<0>(after_log[i]));
        EXPECT_EQ(std::get<1>(before_log[i]), std::get<1>(after_log[i]));
        EXPECT_NEAR(std::get<2>(before_log[i]), std::get<2>(after_log[i]), 1e-8);
        EXPECT_NEAR(std::get<3>(before_log[i]), std::get<3>(after_log[i]), 1e-8);
    }
}

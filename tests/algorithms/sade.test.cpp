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
#include <pagmo/algorithms/sade.hpp>
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

TEST(sade_test, construction_test)
{
    sade user_algo{53u, 2u, 1u, 1e-6, 1e-6, false, 23u};
    EXPECT_TRUE(user_algo.get_verbosity() == 0u);
    EXPECT_TRUE(user_algo.get_seed() == 23u);
    EXPECT_TRUE((user_algo.get_log() == sade::log_type{}));

    EXPECT_THROW((sade{53u, 0u, 1u, 1e-6, 1e-6, false, 23u}), invalid_parameter_error);
    EXPECT_THROW((sade{53u, 23u, 1u, 1e-6, 1e-6, false, 23u}), invalid_parameter_error);
    EXPECT_THROW((sade{53u, 2u, 0u, 1e-6, 1e-6, false, 23u}), invalid_parameter_error);
    EXPECT_THROW((sade{53u, 2u, 3u, 1e-6, 1e-6, false, 23u}), invalid_parameter_error);
}

TEST(sade_test, evolve_test)
{
    // Here we only test that evolution is deterministic if the
    // seed is controlled for all variants
    {
        problem prob{rosenbrock{25u}};
        population pop1{prob, 15u, 23u};
        population pop2{prob, 15u, 23u};
        population pop3{prob, 15u, 23u};

        for (unsigned j = 1u; j <= 2u; ++j) {
            for (unsigned i = 1u; i <= 18u; ++i) {
                sade user_algo1{10u, i, j, 1e-6, 1e-6, false, 23u};
                user_algo1.set_verbosity(1u);
                pop1 = user_algo1.evolve(pop1);

                EXPECT_TRUE(user_algo1.get_log().size() > 0u);

                sade user_algo2{10u, i, j, 1e-6, 1e-6, false, 23u};
                user_algo2.set_verbosity(1u);
                pop2 = user_algo2.evolve(pop2);

                EXPECT_TRUE(user_algo1.get_log() == user_algo2.get_log());

                user_algo2.set_seed(23u);
                pop3 = user_algo2.evolve(pop3);

                EXPECT_TRUE(user_algo1.get_log() == user_algo2.get_log());
            }
        }
    }
    // Here we check that the exit condition of ftol and xtol actually provoke an exit within 300u gen (rosenbrock{2} is
    // used)
    {
        sade user_algo{300u, 2, 1, 1e-3, 1e-16, false, 23u};
        user_algo.set_verbosity(1u);
        problem prob{rosenbrock{2u}};
        population pop{prob, 20u, 23u};
        pop = user_algo.evolve(pop);
        EXPECT_TRUE(user_algo.get_log().size() < 5000u);
    }
    {
        sade user_algo{300u, 2, 1, 1e-16, 1e-3, false, 23u};
        user_algo.set_verbosity(1u);
        problem prob{rosenbrock{2u}};
        population pop{prob, 20u, 23u};
        pop = user_algo.evolve(pop);
        EXPECT_TRUE(user_algo.get_log().size() < 300u);
    }

    // We then check that the evolve throws if called on unsuitable problems
    EXPECT_THROW(sade{10u}.evolve(population{problem{rosenbrock{}}, 6u}), insufficient_population_error);
    EXPECT_THROW(sade{10u}.evolve(population{problem{zdt{}}, 15u}), incompatible_problem_error);
    EXPECT_THROW(sade{10u}.evolve(population{problem{hock_schittkowski_71{}}, 15u}), incompatible_problem_error);
    EXPECT_THROW(sade{10u}.evolve(population{problem{inventory{}}, 15u}), incompatible_problem_error);
    // And a clean exit for 0 generations
    population pop{rosenbrock{25u}, 10u};
    EXPECT_TRUE(sade{0u}.evolve(pop).get_x()[0] == pop.get_x()[0]);
}

TEST(sade_test, setters_getters_test)
{
    sade user_algo{10000000u, 2, 1, 1e-6, 1e-6, false, 23u};
    user_algo.set_verbosity(23u);
    EXPECT_TRUE(user_algo.get_verbosity() == 23u);
    user_algo.set_seed(23u);
    EXPECT_TRUE(user_algo.get_seed() == 23u);
    EXPECT_TRUE(user_algo.get_name().find("Self-adaptive") != std::string::npos);
    EXPECT_TRUE(user_algo.get_extra_info().find("Self adaptation variant") != std::string::npos);
    EXPECT_NO_THROW(user_algo.get_log());
}

TEST(sade_test, serialization_test)
{
    // Make one evolution
    problem prob{rosenbrock{2u}};
    population pop{prob, 15u, 23u};
    algorithm algo{sade{10000000u, 2, 1, 1e-3, 1e-3, false, 23u}};
    algo.set_verbosity(1u);
    pop = algo.evolve(pop);

    // Store the string representation of p.
    std::stringstream ss;
    auto before_text = lexical_cast<std::string>(algo);
    auto before_log = algo.extract<sade>()->get_log();
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
    auto after_log = algo.extract<sade>()->get_log();
    EXPECT_EQ(before_text, after_text);
    EXPECT_TRUE(before_log == after_log);
    // so we implement a close check
    EXPECT_TRUE(before_log.size() > 0u);
    for (auto i = 0u; i < before_log.size(); ++i) {
        EXPECT_EQ(std::get<0>(before_log[i]), std::get<0>(after_log[i]));
        EXPECT_EQ(std::get<1>(before_log[i]), std::get<1>(after_log[i]));
        EXPECT_NEAR(std::get<2>(before_log[i]), std::get<2>(after_log[i]), 1e-8);
        EXPECT_NEAR(std::get<3>(before_log[i]), std::get<3>(after_log[i]), 1e-8);
        EXPECT_NEAR(std::get<4>(before_log[i]), std::get<4>(after_log[i]), 1e-8);
        EXPECT_NEAR(std::get<5>(before_log[i]), std::get<5>(after_log[i]), 1e-8);
        EXPECT_NEAR(std::get<6>(before_log[i]), std::get<6>(after_log[i]), 1e-8);
    }
}

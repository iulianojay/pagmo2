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
#include <limits> //  std::numeric_limits<double>::infinity();
#include <pagmo/utils/cast.hpp>
#include <string>

#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/pso.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problems/hock_schittkowski_71.hpp>
#include <pagmo/problems/inventory.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/problems/zdt.hpp>
#include <pagmo/rng.hpp>
#include <pagmo/exceptions.hpp>

using namespace pagmo;

TEST(pso_test, construction)
{
    EXPECT_NO_THROW(pso{});
    pso user_algo{100, 0.79, 2., 2., 0.1, 5u, 2u, 4u, false, 23u};
    EXPECT_TRUE(user_algo.get_verbosity() == 0u);
    EXPECT_TRUE(user_algo.get_seed() == 23u);
    EXPECT_TRUE((user_algo.get_log() == pso::log_type{}));

    EXPECT_NO_THROW((pso{100, 0.79, 2., 2., 0.1, 5u, 2u, 4u, false, 23u}));

    EXPECT_THROW((pso{100, -0.79, 2., 2., 0.1, 5u, 2u, 4u, false, 23u}), invalid_parameter_error);
    EXPECT_THROW((pso{100, 2.3, 2., 2., 0.1, 5u, 2u, 4u, false, 23u}), invalid_parameter_error);

    EXPECT_THROW((pso{100, 0.79, -1., 2., 0.1, 5u, 2u, 4u, false, 23u}), invalid_parameter_error);
    EXPECT_THROW((pso{100, 0.79, 2., -1., 0.1, 5u, 2u, 4u, false, 23u}), invalid_parameter_error);
    EXPECT_THROW((pso{100, 0.79, 5., 2., 0.1, 5u, 2u, 4u, false, 23u}), invalid_parameter_error);
    EXPECT_THROW((pso{100, 0.79, 2., 5., 0.1, 5u, 2u, 4u, false, 23u}), invalid_parameter_error);

    EXPECT_THROW((pso{100, 0.79, 2., 2., -2.3, 5u, 2u, 4u, false, 23u}), invalid_parameter_error);
    EXPECT_THROW((pso{100, 0.79, 2., 2., 1.1, 5u, 2u, 4u, false, 23u}), invalid_parameter_error);

    EXPECT_THROW((pso{100, 0.79, 2., 2., 0.1, 8u, 2u, 4u, false, 23u}), invalid_parameter_error);
    EXPECT_THROW((pso{100, 0.79, 2., 2., 0.1, 0u, 2u, 4u, false, 23u}), invalid_parameter_error);

    EXPECT_THROW((pso{100, 0.79, 2., 2., 0.1, 5u, 6u, 4u, false, 23u}), invalid_parameter_error);
    EXPECT_THROW((pso{100, 0.79, 2., 2., 0.1, 5u, 0u, 4u, false, 23u}), invalid_parameter_error);

    EXPECT_THROW((pso{100, 0.79, 2., 2., 0.1, 5u, 2u, 0u, false, 23u}), invalid_parameter_error);
}

TEST(pso_test, evolve_test)
{
    // We then check that the evolve throws if called on unsuitable problems
    EXPECT_THROW(pso{10u}.evolve(population{problem{rosenbrock{}}}), insufficient_population_error);
    EXPECT_THROW(pso{10u}.evolve(population{problem{zdt{}}, 15u}), incompatible_problem_error);
    EXPECT_THROW(pso{10u}.evolve(population{problem{hock_schittkowski_71{}}, 15u}), incompatible_problem_error);
    EXPECT_THROW(pso{10u}.evolve(population{problem{inventory{}}, 15u}), incompatible_problem_error);
    // And a clean exit for 0 generations
    population pop{rosenbrock{2u}, 20u};
    EXPECT_TRUE(pso{0u}.evolve(pop).get_x()[0] == pop.get_x()[0]);

    // We check that evolution is deterministic if the
    // seed is controlled and for all algorithmic variants:
    for (unsigned variant = 1u; variant <= 6u; ++variant) {
        for (unsigned neighb_type = 1u; neighb_type <= 4u; ++neighb_type) {
            problem prob{rosenbrock{10u}};
            population pop1{prob, 5u, 23u};
            pso user_algo1{10u, 0.79, 2., 2., 0.1, variant, neighb_type, 4u, false, 23u};
            user_algo1.set_verbosity(1u);
            pop1 = user_algo1.evolve(pop1);

            population pop2{prob, 5u, 23u};
            pso user_algo2{10u, 0.79, 2., 2., 0.1, variant, neighb_type, 4u, false, 23u};
            user_algo2.set_verbosity(1u);
            pop2 = user_algo2.evolve(pop2);
            EXPECT_TRUE(user_algo1.get_log() == user_algo2.get_log());

            population pop3{prob, 5u, 23u};
            user_algo2.set_seed(23u);
            pop3 = user_algo2.evolve(pop3);
            EXPECT_TRUE(user_algo1.get_log() == user_algo2.get_log());
        }
    }
    // And with active memory
    for (unsigned variant = 1u; variant <= 6u; ++variant) {
        for (unsigned neighb_type = 1u; neighb_type <= 4u; ++neighb_type) {
            problem prob{rosenbrock{10u}};
            population pop1{prob, 5u, 23u};
            pso user_algo1{10u, 0.79, 2., 2., 0.1, variant, neighb_type, 4u, true, 23u};
            user_algo1.set_verbosity(1u);
            pop1 = user_algo1.evolve(pop1);

            population pop2{prob, 5u, 23u};
            pso user_algo2{10u, 0.79, 2., 2., 0.1, variant, neighb_type, 4u, true, 23u};
            user_algo2.set_verbosity(1u);
            pop2 = user_algo2.evolve(pop2);
            EXPECT_TRUE(user_algo1.get_log() == user_algo2.get_log());

            population pop3{prob, 5u, 23u};
            pso user_algo3{10u, 0.79, 2., 2., 0.1, variant, neighb_type, 4u, true, 0u};
            user_algo3.set_verbosity(1u);
            user_algo3.set_seed(23u);
            pop3 = user_algo3.evolve(pop3);
            EXPECT_TRUE(user_algo1.get_log() == user_algo3.get_log());
        }
    }
}
TEST(pso_test, setters_getters_test)
{
    pso user_algo{5000u, 0.79, 2., 2., 0.1, 5u, 2u, 4u, false, 23u};
    user_algo.set_verbosity(200u);
    EXPECT_TRUE(user_algo.get_verbosity() == 200u);
    user_algo.set_seed(23u);
    EXPECT_TRUE(user_algo.get_seed() == 23u);
    EXPECT_TRUE(user_algo.get_name().find("Particle Swarm") != std::string::npos);
    EXPECT_TRUE(user_algo.get_extra_info().find("Verbosity") != std::string::npos);
    EXPECT_NO_THROW(user_algo.get_log());
}

TEST(pso_test, serialization_test)
{
    // Make one evolution
    problem prob{rosenbrock{25u}};
    population pop{prob, 5u, 23u};
    algorithm algo{pso{500u, 0.79, 2., 2., 0.1, 5u, 2u, 4u, false, 23u}};
    algo.set_verbosity(23u);
    pop = algo.evolve(pop);

    // Store the string representation of p.
    std::stringstream ss;
    auto before_text = lexical_cast<std::string>(algo);
    auto before_log = algo.extract<pso>()->get_log();
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
    auto after_log = algo.extract<pso>()->get_log();
    EXPECT_EQ(before_text, after_text);
    EXPECT_TRUE(before_log == after_log);
    // so we implement a close check
    for (auto i = 0u; i < before_log.size(); ++i) {
        EXPECT_EQ(std::get<0>(before_log[i]), std::get<0>(after_log[i]));
        EXPECT_EQ(std::get<1>(before_log[i]), std::get<1>(after_log[i]));
        EXPECT_NEAR(std::get<2>(before_log[i]), std::get<2>(after_log[i]), 1e-8);
        EXPECT_NEAR(std::get<3>(before_log[i]), std::get<3>(after_log[i]), 1e-8);
        EXPECT_NEAR(std::get<4>(before_log[i]), std::get<4>(after_log[i]), 1e-8);
        EXPECT_NEAR(std::get<5>(before_log[i]), std::get<5>(after_log[i]), 1e-8);
    }
}

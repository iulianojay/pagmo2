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
#include <numeric>
#include <string>

#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/de1220.hpp>
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

TEST(de1220_test, construction_test)
{
    std::vector<unsigned> mutation_variants(18);
    std::iota(mutation_variants.begin(), mutation_variants.end(), 1u);
    de1220 user_algo(53u, mutation_variants, 1u, 1e-6, 1e-6, false, 23u);
    EXPECT_TRUE(user_algo.get_verbosity() == 0u);
    EXPECT_TRUE(user_algo.get_seed() == 23u);
    EXPECT_TRUE((user_algo.get_log() == de1220::log_type{}));

    EXPECT_THROW((de1220{53u, {3u, 5u, 0u, 14u}, 1u, 1e-6, 1e-6, false, 23u}), invalid_parameter_error);
    EXPECT_THROW((de1220{53u, {4u, 5u, 15u, 22u, 7u}, 1u, 1e-6, 1e-6, false, 23u}), invalid_parameter_error);
    EXPECT_THROW((de1220{53u, mutation_variants, 0u, 1e-6, 1e-6, false, 23u}), invalid_parameter_error);
    EXPECT_THROW((de1220{53u, mutation_variants, 3u, 1e-6, 1e-6, false, 23u}), invalid_parameter_error);
}

TEST(de1220_test, evolve_test)
{
    // We consider all variants
    std::vector<unsigned> mutation_variants(18);
    std::iota(mutation_variants.begin(), mutation_variants.end(), 1u);
    // Here we only test that evolution is deterministic if the
    // seed is controlled for all variants
    {
        problem prob{rosenbrock{25u}};
        population pop1{prob, 15u, 23u};
        population pop2{prob, 15u, 23u};
        population pop3{prob, 15u, 23u};

        for (unsigned i = 1u; i <= 2u; ++i) {
            de1220 user_algo1(10u, mutation_variants, i, 1e-6, 1e-6, false, 41u);
            user_algo1.set_verbosity(1u);
            pop1 = user_algo1.evolve(pop1);

            EXPECT_TRUE(user_algo1.get_log().size() > 0u);

            de1220 user_algo2{10u, mutation_variants, i, 1e-6, 1e-6, false, 41u};
            user_algo2.set_verbosity(1u);
            pop2 = user_algo2.evolve(pop2);

            EXPECT_TRUE(user_algo1.get_log() == user_algo2.get_log());

            user_algo2.set_seed(41u);
            pop3 = user_algo2.evolve(pop3);

            EXPECT_TRUE(user_algo1.get_log() == user_algo2.get_log());
        }
    }

    // Here we check that the exit condition of ftol and xtol actually provoke an exit within 5000 gen (rosenbrock{2} is
    // used)
    { // xtol
        de1220 user_algo(300u, mutation_variants, 2, 1e-3, 1e-45, false, 41u);
        user_algo.set_verbosity(1u);
        problem prob{rosenbrock{2u}};
        population pop{prob, 20u, 23u};
        pop = user_algo.evolve(pop);
        EXPECT_TRUE(user_algo.get_log().size() < 300u);
    }
    { // ftol
        de1220 user_algo(300u, mutation_variants, 1, 1e-45, 1e-3, false, 41u);
        user_algo.set_verbosity(1u);
        problem prob{rosenbrock{2u}};
        population pop{prob, 20u, 23u};
        pop = user_algo.evolve(pop);
        EXPECT_TRUE(user_algo.get_log().size() < 300u);
    }

    // We then check that the evolve throws if called on unsuitable problems
    EXPECT_THROW(de1220{10u}.evolve(population{problem{rosenbrock{}}, 6u}), insufficient_population_error);
    EXPECT_THROW(de1220{10u}.evolve(population{problem{zdt{}}, 15u}), incompatible_problem_error);
    EXPECT_THROW(de1220{10u}.evolve(population{problem{hock_schittkowski_71{}}, 15u}), incompatible_problem_error);
    EXPECT_THROW(de1220{10u}.evolve(population{problem{inventory{}}, 15u}), incompatible_problem_error);
    // And a clean exit for 0 generations
    population pop{rosenbrock{25u}, 10u};
    EXPECT_TRUE(de1220{0u}.evolve(pop).get_x()[0] == pop.get_x()[0]);
}

TEST(de1220_test, setters_getters_test)
{
    // We consider all variants
    std::vector<unsigned> mutation_variants(18);
    std::iota(mutation_variants.begin(), mutation_variants.end(), 1u);
    de1220 user_algo(10000u, mutation_variants, 1, 1e-6, 1e-6, false, 41u);
    user_algo.set_verbosity(23u);
    EXPECT_TRUE(user_algo.get_verbosity() == 23u);
    user_algo.set_seed(23u);
    EXPECT_TRUE(user_algo.get_seed() == 23u);
    EXPECT_TRUE(user_algo.get_name().find("1220") != std::string::npos);
    EXPECT_TRUE(user_algo.get_extra_info().find("Allowed variants") != std::string::npos);
    EXPECT_NO_THROW(user_algo.get_log());
}

TEST(de1220_test, serialization_test)
{
    // Make one evolution
    problem prob{rosenbrock{2u}};
    population pop{prob, 15u, 23u};
    std::vector<unsigned> mutation_variants(18);
    std::iota(mutation_variants.begin(), mutation_variants.end(), 1u);
    algorithm algo(de1220{10000u, mutation_variants, 1, 1e-6, 1e-6, false, 41u});
    algo.set_verbosity(1u);
    pop = algo.evolve(pop);

    // Store the string representation of p.
    std::stringstream ss;
    auto before_text = lexical_cast<std::string>(algo);
    auto before_log = algo.extract<de1220>()->get_log();
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
    auto after_log = algo.extract<de1220>()->get_log();
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
        EXPECT_EQ(std::get<5>(before_log[i]), std::get<5>(after_log[i]));
        EXPECT_NEAR(std::get<6>(before_log[i]), std::get<6>(after_log[i]), 1e-8);
        EXPECT_NEAR(std::get<7>(before_log[i]), std::get<7>(after_log[i]), 1e-8);
    }
}

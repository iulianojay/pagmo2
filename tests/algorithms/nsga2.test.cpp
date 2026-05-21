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

#include <algorithm>
#include <iostream>
#include <string>

#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/nsga2.hpp>
#include <pagmo/exceptions.hpp>
#include <pagmo/io.hpp>
#include <pagmo/problems/dtlz.hpp>
#include <pagmo/problems/hock_schittkowski_71.hpp>
#include <pagmo/problems/inventory.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/problems/zdt.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>

using namespace pagmo;

TEST(nsga2_test, nsga2_algorithm_construction)
{
    nsga2 user_algo{1u, 0.95, 10., 0.01, 50., 32u};
    EXPECT_NO_THROW(nsga2{});
    EXPECT_TRUE(user_algo.get_verbosity() == 0u);
    EXPECT_TRUE(user_algo.get_seed() == 32u);
    // EXPECT_TRUE((user_algo.get_log() == moead::log_type{}));

    // Check the throws
    // Wrong cr
    EXPECT_THROW((nsga2{1u, 1., 10., 0.01, 50., 32u}), invalid_parameter_error);
    EXPECT_THROW((nsga2{1u, -1., 10., 0.01, 50., 32u}), invalid_parameter_error);
    // Wrong m
    EXPECT_THROW((nsga2{1u, .95, 10., 1.1, 50., 32u}), invalid_parameter_error);
    EXPECT_THROW((nsga2{1u, .95, 10., -1.1, 50., 32u}), invalid_parameter_error);
    // Wrong eta_m
    EXPECT_THROW((nsga2{1u, .95, 100.1, 0.01, 50., 32u}), invalid_parameter_error);
    EXPECT_THROW((nsga2{1u, .95, .98, 0.01, 50., 32u}), invalid_parameter_error);
    // Wrong eta_m
    EXPECT_THROW((nsga2{1u, .95, 10., 0.01, 100.1, 32u}), invalid_parameter_error);
    EXPECT_THROW((nsga2{1u, .95, 10., 0.01, .98, 32u}), invalid_parameter_error);
}

struct mo_equal_bounds {
    /// Fitness
    vector_double fitness(const vector_double &) const
    {
        return {0., 0.};
    }
    vector_double::size_type get_nobj() const
    {
        return 2u;
    }
    /// Problem bounds
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{0., 0.}, {1., 0.}};
    }
};

TEST(nsga2_test, nsga2_evolve_test)
{
    // We check that the problem is checked to be suitable
    // Some bound is equal
    EXPECT_THROW(nsga2{10u}.evolve(population{problem{mo_equal_bounds{}}, 0u}), incompatible_problem_error);
    // stochastic
    EXPECT_THROW((nsga2{}.evolve(population{inventory{}, 5u, 23u})), incompatible_problem_error);
    // constrained prob
    EXPECT_THROW((nsga2{}.evolve(population{hock_schittkowski_71{}, 5u, 23u})), incompatible_problem_error);
    // single objective prob
    EXPECT_THROW((nsga2{}.evolve(population{rosenbrock{}, 5u, 23u})), incompatible_problem_error);
    // wrong population size
    EXPECT_THROW((nsga2{}.evolve(population{zdt{}, 3u, 23u})), insufficient_population_error);
    EXPECT_THROW((nsga2{}.evolve(population{zdt{}, 50u, 23u})), insufficient_population_error);

    // We check for deterministic behaviour if the seed is controlled
    // we treat the last three components of the decision vector as integers
    // to trigger all cases
    dtlz udp{1u, 10u, 3u};

    population pop1{udp, 52u, 23u};
    population pop2{udp, 52u, 23u};
    population pop3{udp, 52u, 23u};

    nsga2 user_algo1{10u, 0.95, 10., 0.01, 50., 32u};
    user_algo1.set_verbosity(1u);
    pop1 = user_algo1.evolve(pop1);

    EXPECT_TRUE(user_algo1.get_log().size() > 0u);

    nsga2 user_algo2{10u, 0.95, 10., 0.01, 50., 32u};
    user_algo2.set_verbosity(1u);
    pop2 = user_algo2.evolve(pop2);

    EXPECT_TRUE(user_algo1.get_log() == user_algo2.get_log());

    user_algo2.set_seed(32u);
    pop3 = user_algo2.evolve(pop3);

    EXPECT_TRUE(user_algo1.get_log() == user_algo2.get_log());

    // We evolve for many-objectives and trigger the output with the ellipses
    udp = dtlz{1u, 12u, 7u};
    population pop4{udp, 52u, 23u};
    pop4 = user_algo2.evolve(pop4);
}

TEST(nsga2_test, nsga2_setters_getters_test)
{
    nsga2 user_algo{1u, 0.95, 10., 0.01, 50., 32u};
    user_algo.set_verbosity(200u);
    EXPECT_TRUE(user_algo.get_verbosity() == 200u);
    user_algo.set_seed(23456u);
    EXPECT_TRUE(user_algo.get_seed() == 23456u);
    EXPECT_TRUE(user_algo.get_name().find("NSGA-II") != std::string::npos);
    EXPECT_TRUE(user_algo.get_extra_info().find("Verbosity") != std::string::npos);
    // EXPECT_NO_THROW(user_algo.get_log());
}

TEST(nsga2_test, nsga2_zdt5_test)
{
    algorithm algo{nsga2(100u, 0.95, 10., 0.01, 50., 32u)};
    algo.set_verbosity(10u);
    algo.set_seed(23456u);
    population pop{zdt(5u, 10u), 20u, 32u};
    pop = algo.evolve(pop);
    for (decltype(pop.size()) i = 0u; i < pop.size(); ++i) {
        auto x = pop.get_x()[i];
        EXPECT_TRUE(std::all_of(x.begin(), x.end(), [](double el) { return (el == std::floor(el)); }));
    }
}

TEST(nsga2_test, nsga2_serialization_test)
{
    // Make one evolution
    problem prob{zdt{1u, 30u}};
    population pop{prob, 40u, 23u};
    algorithm algo{nsga2{10u, 0.95, 10., 0.01, 50., 32u}};
    algo.set_verbosity(1u);
    algo.set_seed(1234u);
    pop = algo.evolve(pop);

    // Store the string representation of p.
    std::stringstream ss;
    auto before_text = lexical_cast<std::string>(algo);
    auto before_log = algo.extract<nsga2>()->get_log();
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
    auto after_log = algo.extract<nsga2>()->get_log();
    EXPECT_EQ(before_text, after_text);
    EXPECT_TRUE(before_log == after_log);
    // so we implement a close check
    EXPECT_TRUE(before_log.size() > 0u);
    for (auto i = 0u; i < before_log.size(); ++i) {
        EXPECT_EQ(std::get<0>(before_log[i]), std::get<0>(after_log[i]));
        EXPECT_EQ(std::get<1>(before_log[i]), std::get<1>(after_log[i]));
        for (auto j = 0u; j < 2u; ++j) {
            EXPECT_NEAR(std::get<2>(before_log[i])[j], std::get<2>(after_log[i])[j], 1e-8);
        }
    }
}

TEST(nsga2_test, bfe_usage_test)
{
    // 1 - Algorithm with bfe disabled
    problem prob{dtlz(1, 10, 2)};
    nsga2 uda1{nsga2{10}};
    uda1.set_verbosity(1u);
    uda1.set_seed(23u);
    // 2 - Instantiate
    algorithm algo1{uda1};

    // 3 - Instantiate populations
    population pop{prob, 24, 32u};
    population pop1{prob, 24, 456u};
    population pop2{prob, 24, 67345u};

    // 4 - Evolve the population
    pop1 = algo1.evolve(pop);

    // 5 - new algorithm that is bfe enabled
    nsga2 uda2{nsga2{10}};
    uda2.set_verbosity(1u);
    uda2.set_seed(23u);
    uda2.set_bfe(bfe{}); // This will use the default bfe.
    // 6 - Instantiate a pagmo algorithm
    algorithm algo2{uda2};

    // 7 - Evolve the population
    pop2 = algo2.evolve(pop);
    EXPECT_TRUE(algo1.extract<nsga2>()->get_log() == algo2.extract<nsga2>()->get_log());
}

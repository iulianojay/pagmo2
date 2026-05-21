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
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <pagmo/batch_evaluators/thread_bfe.hpp>
#include <pagmo/bfe.hpp>
#include <pagmo/exceptions.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/problems/hock_schittkowski_71.hpp>
#include <pagmo/problems/inventory.hpp>
#include <pagmo/problems/null_problem.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/problems/zdt.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>

using namespace pagmo;

static inline std::string pop_to_string(const population &pop)
{
    std::stringstream ss;
    ss << pop;
    return ss.str();
}

TEST(population_test, population_construction_test)
{
    unsigned seed = 123;
    population pop1{};
    population pop2{problem{zdt{1, 5}}, 2, seed};
    population pop3{problem{zdt{2, 5}}, 2, seed};

    // We check that the number of individuals is as expected
    EXPECT_TRUE(pop1.size() == 0u);
    EXPECT_TRUE(pop2.size() == 2u);
    EXPECT_TRUE(pop3.size() == 2u);
    // We check population's individual chromosomes and IDs are the same
    // as the random seed was (and the problem dimension), while
    // fitness vectors were different as the problem is
    EXPECT_TRUE(pop2.get_ID() == pop3.get_ID());
    EXPECT_TRUE(pop2.get_x() == pop3.get_x());
    EXPECT_TRUE(pop2.get_f() != pop3.get_f());
    // We check that the seed has been set correctly
    EXPECT_TRUE(pop2.get_seed() == seed);

    // We test the generic constructor
    population pop4{zdt{2, 5}, 2, seed};
    EXPECT_TRUE(pop4.get_ID() == pop3.get_ID());
    EXPECT_TRUE(pop4.get_x() == pop3.get_x());
    EXPECT_TRUE(pop4.get_f() == pop3.get_f());
    population pop5{zdt{1, 5}, 2, seed};
    EXPECT_TRUE(pop2.get_ID() == pop5.get_ID());
    EXPECT_TRUE(pop2.get_x() == pop5.get_x());
    EXPECT_TRUE(pop2.get_f() == pop5.get_f());

    // Check copy/move semantics.
    population pop_a{problem{zdt{2, 5}}, 2, 20};
    population pop_b{pop_a};
    EXPECT_EQ(pop_to_string(pop_a), pop_to_string(pop_b));
    population pop_c;
    pop_c = pop_b;
    EXPECT_EQ(pop_to_string(pop_a), pop_to_string(pop_c));
    population pop_d{std::move(pop_c)};
    EXPECT_EQ(pop_to_string(pop_a), pop_to_string(pop_d));
    population pop_e;
    pop_e = std::move(pop_b);
    EXPECT_EQ(pop_to_string(pop_a), pop_to_string(pop_e));
    // Try to revive moved-from objects.
    pop_c = pop_e;
    EXPECT_EQ(pop_to_string(pop_a), pop_to_string(pop_c));
    pop_b = std::move(pop_e);
    EXPECT_EQ(pop_to_string(pop_a), pop_to_string(pop_b));

    // Self assignments.
    pop_a = pop_b;
    pop_a = *&pop_a;
    EXPECT_EQ(pop_to_string(pop_a), pop_to_string(pop_b));
#if !defined(__clang__)
    pop_a = std::move(pop_a);
    EXPECT_EQ(pop_to_string(pop_a), pop_to_string(pop_b));
#endif

    // Check constructability.
    EXPECT_TRUE((!std::is_constructible<population, int>::value));
    EXPECT_TRUE((!std::is_constructible<population, int &>::value));
    EXPECT_TRUE((!std::is_constructible<population, const int &>::value));
    EXPECT_TRUE((!std::is_constructible<population, std::string>::value));
    EXPECT_TRUE((std::is_constructible<population, null_problem>::value));
    EXPECT_TRUE((std::is_constructible<population, null_problem &>::value));
    EXPECT_TRUE((std::is_constructible<population, null_problem &&>::value));
    EXPECT_TRUE((std::is_constructible<population, const null_problem &>::value));
    EXPECT_TRUE((std::is_constructible<population, const null_problem>::value));
    EXPECT_TRUE((std::is_constructible<population, problem>::value));
    EXPECT_TRUE((std::is_constructible<population, problem &>::value));
    EXPECT_TRUE((std::is_constructible<population, problem &&>::value));
    EXPECT_TRUE((std::is_constructible<population, const problem &>::value));
    EXPECT_TRUE((std::is_constructible<population, const problem>::value));
}

TEST(population_test, population_copy_constructor_test)
{
    population pop1{problem{rosenbrock{5}}, 10u};
    population pop2(pop1);
    EXPECT_TRUE(pop2.get_ID() == pop1.get_ID());
    EXPECT_TRUE(pop2.get_x() == pop1.get_x());
    EXPECT_TRUE(pop2.get_f() == pop1.get_f());
}

struct malformed {
    vector_double fitness(const vector_double &) const
    {
        return {0.5};
    }
    vector_double::size_type get_nobj() const
    {
        return 2u;
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{0.}, {1.}};
    }
};

TEST(population_test, population_push_back_test)
{
    // Create an empty population
    population pop{problem{zdt{1u, 30u}}};
    // We fill it with a few individuals and check the size growth
    for (unsigned i = 0u; i < 5u; ++i) {
        EXPECT_TRUE(pop.size() == i);
        EXPECT_TRUE(pop.get_f().size() == i);
        EXPECT_TRUE(pop.get_x().size() == i);
        EXPECT_TRUE(pop.get_ID().size() == i);
        pop.push_back(vector_double(30u, 0.5));
    }
    // We check the fitness counter
    EXPECT_TRUE(pop.get_problem().get_fevals() == 5u);
    // We check important undefined throws
    // 1 - Cannot push back the wrong decision vector dimension
    EXPECT_THROW(pop.push_back(vector_double(28u, 0.5)), dimension_mismatch_error);
    // 2 - Malformed problem. The user declares 2 objectives but returns something else
    population pop2{problem{malformed{}}};
    EXPECT_THROW(pop2.push_back({1.}), dimension_mismatch_error);
    // 3 - Consistency checks on the second push_back() overload.
    population pop3{problem{zdt{1u, 30u}}};
    EXPECT_THROW(pop3.push_back({}, {}), dimension_mismatch_error);
    EXPECT_THROW(pop3.push_back(vector_double(30u, 0.5), {}), dimension_mismatch_error);
    EXPECT_THROW(pop3.push_back(vector_double(30u, 0.5), {0.}), dimension_mismatch_error);
    EXPECT_THROW(pop3.push_back(vector_double(30u, 0.5), {0., 0., 0.}), dimension_mismatch_error);
}

TEST(population_test, population_random_decision_vector_test)
{
    // Create an empty population
    population pop{problem{}};
    auto bounds = pop.get_problem().get_bounds();
    // Generate a random decision_vector
    auto x = pop.random_decision_vector();
    // Check that the decision_vector is indeed within bounds
    for (decltype(x.size()) i = 0u; i < x.size(); ++i) {
        EXPECT_TRUE(x[i] < bounds.second[i]);
        EXPECT_TRUE(x[i] >= bounds.first[i]);
    }
}

TEST(population_test, population_best_worst_test)
{
    // Test throw
    {
        population pop{problem{zdt{}}, 2};
        population pop2{problem{}, 0u};
        EXPECT_THROW(pop.best_idx(), incompatible_problem_error);
        EXPECT_THROW(pop.worst_idx(), incompatible_problem_error);
        EXPECT_THROW(pop2.best_idx(), empty_collection_error);
        EXPECT_THROW(pop2.worst_idx(), empty_collection_error);
    }
    // Test on single objective
    {
        population pop{problem{rosenbrock{2}}};
        pop.push_back({0.5, 0.5});
        pop.push_back(pop.get_problem().extract<rosenbrock>()->best_known());
        EXPECT_TRUE(pop.worst_idx() == 0u);
        EXPECT_TRUE(pop.best_idx() == 1u);
    }
    // Test on constrained
    {
        population pop{problem{hock_schittkowski_71{}}};
        pop.push_back({1.5, 1.5, 1.5, 1.5});
        pop.push_back(pop.get_problem().extract<hock_schittkowski_71>()->best_known());
        EXPECT_TRUE(pop.worst_idx(1e-5) == 0u); // tolerance matter here!!!
        EXPECT_TRUE(pop.best_idx(1e-5) == 1u);
    }
}

TEST(population_test, population_setters_test)
{
    population pop{problem{}, 2};
    // Test throw
    EXPECT_THROW(pop.set_xf(2, {3}, {1, 2, 3}), index_error);           // index invalid
    EXPECT_THROW(pop.set_xf(1, {3, 2}, {1}), dimension_mismatch_error); // chromosome invalid
    EXPECT_THROW(pop.set_xf(1, {3}, {1, 2}), dimension_mismatch_error); // fitness invalid
    // Test set_xf
    pop.set_xf(0, {3}, {1});
    EXPECT_TRUE((pop.get_x()[0] == vector_double{3}));
    EXPECT_TRUE((pop.get_f()[0] == vector_double{1}));
    // Test set_x
    pop.set_x(0, {1.2});
    EXPECT_TRUE((pop.get_x()[0] == vector_double{1.2}));
    EXPECT_TRUE(pop.get_f()[0] == pop.get_problem().fitness({1.2})); // works as counters are marked mutable
}

TEST(population_test, population_getters_test)
{
    population pop{problem{}, 1, 1234u};
    pop.set_xf(0, {3}, {1});
    // Test
    EXPECT_TRUE((pop.get_f()[0] == vector_double{1}));
    EXPECT_TRUE(pop.get_seed() == 1234u);
    EXPECT_NO_THROW(pop.get_ID());
    // Streaming operator is tested to contain the problem stream
    auto pop_string = lexical_cast<std::string>(pop);
    auto prob_string = lexical_cast<std::string>(pop.get_problem());
    EXPECT_TRUE(pop_string.find(prob_string) != std::string::npos);
}

TEST(population_test, population_champion_test)
{
    // Unconstrained case
    {
        population pop{problem{rosenbrock{2u}}};
        // Upon construction of an empty population the Champion is empty
        EXPECT_TRUE((pop.champion_x() == vector_double{}));
        EXPECT_TRUE((pop.champion_f() == vector_double{}));
        // We push back the origin, in Rosenbrock this has a fitness of 1.
        pop.push_back({0., 0.});
        EXPECT_TRUE((pop.champion_x() == vector_double{0., 0.}));
        EXPECT_TRUE((pop.champion_f() == vector_double{1.}));
        // We push back .1,.1, in Rosenbrock this has a fitness of 1.62 and thus should not trigger the champion update
        pop.push_back({0.1, 0.1});
        EXPECT_TRUE((pop.champion_x() == vector_double{0., 0.}));
        EXPECT_TRUE((pop.champion_f() == vector_double{1.}));
        // We push back 0.01,0.01, in Rosenbrock this has a fitness of 0.989901 and thus should trigger the champion
        // update
        pop.push_back({0.01, 0.01});
        EXPECT_TRUE((pop.champion_x() == vector_double{0.01, 0.01}));
        EXPECT_NEAR(pop.champion_f()[0], 0.989901, 1e-6);
        // We set the chromosome of this last individual to something worse, the champion does not change
        pop.set_x(2u, {0.1, 0.1});
        EXPECT_TRUE((pop.champion_x() == vector_double{0.01, 0.01}));
        EXPECT_NEAR(pop.champion_f()[0], 0.989901, 1e-6);
        // We set the chromosome of this last individual to something better, the champion does change
        pop.set_xf(2u, {0.123, 0.123}, {0.12});
        EXPECT_TRUE((pop.champion_x() == vector_double{0.123, 0.123}));
        EXPECT_TRUE((pop.champion_f() == vector_double{0.12}));
    }
    // Constrained case
    {
        population pop{problem{hock_schittkowski_71{}}};
        // Upon construction of an empty population the Champion is empty
        EXPECT_TRUE((pop.champion_x() == vector_double{}));
        EXPECT_TRUE((pop.champion_f() == vector_double{}));
        // We push back 1.1,1.1,.. in hock_schittkowski_71 this has a fitness of [  5.093, -35.16, 23.5359]
        pop.push_back({1.1, 1.1, 1.1, 1.1});
        auto ch = pop.champion_f();
        EXPECT_TRUE((pop.champion_x() == vector_double{1.1, 1.1, 1.1, 1.1}));
        // We push back all ones, in hock_schittkowski_71 this has a fitness of [ 4., -36., 24.] and does not trigger a
        // champion update
        pop.push_back({1., 1., 1., 1.});
        EXPECT_TRUE((pop.champion_x() == vector_double{1.1, 1.1, 1.1, 1.1}));
        EXPECT_TRUE((pop.champion_f() == ch));
        // We push back 2.1,2.1, in hock_schittkowski_71 this has a fitness of [29.883 ,-22.36, 5.5519] and triggers a
        // champion update
        pop.push_back({2.1, 2.1, 2.1, 2.1});
        EXPECT_TRUE((pop.champion_x() == vector_double{2.1, 2.1, 2.1, 2.1}));
        EXPECT_TRUE((pop.champion_f() != ch));
        ch = pop.champion_f();
        // We set the chromosome of this last individual to something worse, the champion does not change
        pop.set_xf(2u, {1.2, 1.3, 1.4, 1.5}, {12., 45., 55.});
        EXPECT_TRUE((pop.champion_x() == vector_double{2.1, 2.1, 2.1, 2.1}));
        EXPECT_TRUE(pop.champion_f() == ch);
        // We set the chromosome of this last individual to something better, the champion does change
        pop.set_xf(2u, {1.2, 1.3, 1.4, 1.5}, {12., 5., -55.});
        EXPECT_TRUE((pop.champion_x() == vector_double{1.2, 1.3, 1.4, 1.5}));
        EXPECT_TRUE((pop.champion_f() == vector_double{12., 5., -55.}));
    }
    // We check that requests to the champion cannot be made if the population
    // contains a problem with more than 1 objective or is stochastic
    population pop_mo{problem{zdt{}}, 2u};
    EXPECT_THROW(pop_mo.champion_f(), incompatible_problem_error);
    EXPECT_THROW(pop_mo.champion_x(), incompatible_problem_error);
    population pop_sto{problem{inventory{12u}}, 2u};
    EXPECT_THROW(pop_sto.champion_f(), incompatible_problem_error);
    EXPECT_THROW(pop_sto.champion_x(), incompatible_problem_error);
}

TEST(population_test, population_serialization_test)
{
    population pop{problem{}, 30, 1234u};
    // Store the string representation of p.
    std::stringstream ss;
    auto before = lexical_cast<std::string>(pop);
    // Now serialize, deserialize and compare the result.
    {
        cereal::BinaryOutputArchive oarchive(ss);
        oarchive(pop);
    }
    // Change the content of p before deserializing.
    pop = population{problem{zdt{5, 20u}}, 30};
    {
        cereal::BinaryInputArchive iarchive(ss);
        iarchive(pop);
    }
    auto after = lexical_cast<std::string>(pop);
    EXPECT_EQ(before, after);
}

struct minlp {
    minlp(vector_double::size_type nix = 0u)
    {
        m_nix = nix;
    }
    vector_double fitness(const vector_double &x) const
    {
        return {std::sin(x[0] * x[1] * x[2]), x[0] + x[1] + x[2], x[0] * x[1] + x[1] * x[2] - x[0] * x[2]};
    }
    vector_double::size_type get_nobj() const
    {
        return 3u;
    }
    vector_double::size_type get_nix() const
    {
        return m_nix;
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{1, -1, 1}, {2, 1, 2}};
    }
    vector_double::size_type m_nix;
};

TEST(population_test, population_minlp_test)
{
    population pop{problem{minlp{2}}, 30, 1234u};
    for (decltype(pop.size()) i = 0u; i < pop.size(); ++i) {
        EXPECT_TRUE(pop.get_x()[i][2] == std::floor(pop.get_x()[i][2]));
        EXPECT_TRUE(pop.get_x()[i][1] == std::floor(pop.get_x()[i][1]));
    }
}

TEST(population_test, population_cout_test)
{
    population pop{problem{rosenbrock{2u}}};
    population pop_sto{problem{inventory{12u}}, 3u};
    population pop_mo{problem{zdt{}}, 3u};
    EXPECT_NO_THROW(std::cout << pop);
    EXPECT_NO_THROW(std::cout << pop_sto);
    EXPECT_NO_THROW(std::cout << pop_mo);
}

TEST(population_test, population_bfe_ctor_test)
{
    // Empty pop test. Use rosenbrock as we know
    // it's fully thread-safe.
    problem prob{rosenbrock{2u}};
    population pop0{prob, bfe{}};
    EXPECT_TRUE(pop0.size() == 0u);
    EXPECT_TRUE(pop0.get_problem().get_fevals() == 0u);

    // Population with 100 individuals, verify that
    // the fitnesses are computed correctly.
    population pop100{rosenbrock{2u}, bfe{}, 100, 42};
    population pop100a{rosenbrock{2u}, 100, 42};
    EXPECT_TRUE(pop100.size() == 100u);
    EXPECT_TRUE(pop100.get_problem().get_fevals() == 100u);
    for (auto i = 0u; i < 100u; ++i) {
        EXPECT_TRUE(pop100.get_f()[i] == prob.fitness(pop100.get_x()[i]));
        EXPECT_TRUE(pop100.get_x()[i] == pop100a.get_x()[i]);
        EXPECT_TRUE(pop100.get_f()[i] == pop100a.get_f()[i]);
        EXPECT_TRUE(pop100.get_ID()[i] == pop100a.get_ID()[i]);
    }
    EXPECT_TRUE(pop100.champion_x() == pop100a.champion_x());
    EXPECT_TRUE(pop100.champion_f() == pop100a.champion_f());

    // Same with 1000 individuals.
    population pop1000{rosenbrock{2u}, bfe{}, 1000, 42};
    population pop1000a{rosenbrock{2u}, 1000, 42};
    EXPECT_TRUE(pop1000.size() == 1000u);
    EXPECT_TRUE(pop1000.get_problem().get_fevals() == 1000u);
    for (auto i = 0u; i < 1000u; ++i) {
        EXPECT_TRUE(pop1000.get_f()[i] == prob.fitness(pop1000.get_x()[i]));
        EXPECT_TRUE(pop1000.get_x()[i] == pop1000a.get_x()[i]);
        EXPECT_TRUE(pop1000.get_f()[i] == pop1000a.get_f()[i]);
        EXPECT_TRUE(pop1000.get_f()[i] == pop1000a.get_f()[i]);
    }
    EXPECT_TRUE(pop1000.champion_x() == pop1000a.champion_x());
    EXPECT_TRUE(pop1000.champion_f() == pop1000a.champion_f());

    // Do a test with a UDBFE.
    pop1000 = population{rosenbrock{2u}, thread_bfe{}, 1000, 42};
    EXPECT_TRUE(pop1000.size() == 1000u);
    EXPECT_TRUE(pop1000.get_problem().get_fevals() == 1000u);
    for (auto i = 0u; i < 1000u; ++i) {
        EXPECT_TRUE(pop1000.get_f()[i] == prob.fitness(pop1000.get_x()[i]));
        EXPECT_TRUE(pop1000.get_x()[i] == pop1000a.get_x()[i]);
        EXPECT_TRUE(pop1000.get_f()[i] == pop1000a.get_f()[i]);
        EXPECT_TRUE(pop1000.get_f()[i] == pop1000a.get_f()[i]);
    }
    EXPECT_TRUE(pop1000.champion_x() == pop1000a.champion_x());
    EXPECT_TRUE(pop1000.champion_f() == pop1000a.champion_f());
}

// Ensure that we use proper floating-point comparisons
// when updating the champion of a population.
TEST(population_test, population_nan_champion)
{
    population pop0{rosenbrock{2u}};
    pop0.push_back({std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()});
    EXPECT_TRUE(std::isnan(pop0.champion_f()[0]));
    pop0.push_back({std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()});
    EXPECT_TRUE(std::isnan(pop0.champion_f()[0]));
    pop0.push_back({1, 2});
    EXPECT_TRUE(!std::isnan(pop0.champion_f()[0]));
    pop0.push_back({std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()});
    EXPECT_TRUE(!std::isnan(pop0.champion_f()[0]));
}

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

#include <limits>
#include <stdexcept>
#include <string>

#include <pagmo/exceptions.hpp>
#include <pagmo/io.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/problems/decompose.hpp>
#include <pagmo/problems/null_problem.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/problems/zdt.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>

using namespace pagmo;

struct mc_01 {
    vector_double fitness(const vector_double &) const
    {
        return {1., 1.};
    }
    vector_double::size_type get_nobj() const
    {
        return 2u;
    }
    vector_double::size_type get_nec() const
    {
        return 1u;
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{0.}, {1.}};
    }
};

TEST(decompose_test, decompose_construction_test)
{
    // First we check directly the two constructors
    problem p0{decompose{}};
    problem p1{decompose{null_problem{2}, {0.5, 0.5}, {0., 0.}, "weighted", false}};

    auto p0_string = lexical_cast<std::string>(p0);
    auto p1_string = lexical_cast<std::string>(p1);

    // We check that the default constructor constructs a problem
    // which has an identical representation to the problem
    // built by the explicit constructor.
    EXPECT_TRUE(p0_string == p1_string);

    // We check the throws
    auto inf = std::numeric_limits<double>::infinity();
    auto nan = std::numeric_limits<double>::quiet_NaN();
    // single objective problem
    EXPECT_THROW(decompose(rosenbrock{}, {0.5, 0.5}, {0., 0.}), incompatible_problem_error);
    // constrained problem
    EXPECT_THROW(decompose(mc_01{}, {0.5, 0.5}, {0., 0.}, "weighted", false), incompatible_problem_error);
    // random decomposition method
    EXPECT_THROW(decompose(zdt{1u, 2u}, {0.5, 0.5}, {0., 0.}, "my_method", false), decomposition_error);
    // wrong length for the weights
    EXPECT_THROW(decompose(zdt{1u, 2u}, {0.5, 0.2, 0.3}, {0., 0.}, "weighted", false), dimension_mismatch_error);
    EXPECT_THROW(decompose(zdt{1u, 2u}, {0.5, inf}, {0., 0.}, "weighted", false), invalid_value_error);
    // wrong length for the reference point
    EXPECT_THROW(decompose(zdt{1u, 2u}, {0.5, 0.5}, {1.}, "weighted", false), dimension_mismatch_error);
    EXPECT_THROW(decompose(zdt{1u, 2u}, {0.5, 0.5}, {0., nan}, "weighted", false), invalid_value_error);
    // weight sum != 1
    EXPECT_THROW(decompose(zdt{1u, 2u}, {0.9, 0.5}, {0., 0.}, "weighted", false), decomposition_error);
    // weight contains negative component
    EXPECT_THROW(decompose(zdt{1u, 2u}, {1.5, -0.5}, {0., 0.}, "weighted", false), decomposition_error);

    print(p1);
}

TEST(decompose_test, decompose_integration_into_problem_test)
{
    problem p{decompose{zdt{1u, 2u}, {0.5, 0.5}, {0., 0.}, "weighted", false}};
    EXPECT_TRUE(p.has_gradient() == false);
    EXPECT_TRUE(p.has_hessians() == false);
    EXPECT_TRUE(p.get_nobj() == 1u);
    EXPECT_THROW(p.gradient({1, 2}), not_implemented_error);
    EXPECT_THROW(p.hessians({1, 2}), not_implemented_error);
}

TEST(decompose_test, decompose_fitness_test)
{
    problem p{zdt{1u, 2u}};
    vector_double lambda{0.5, 0.5};
    vector_double z{0., 0.};
    problem pdw{decompose{zdt{1u, 2u}, lambda, z, "weighted", false}};
    problem pdtch{decompose{zdt{1u, 2u}, lambda, z, "tchebycheff", false}};
    problem pdbi{decompose{zdt{1u, 2u}, lambda, z, "bi", false}};

    vector_double point{1., 1.};
    auto f = p.fitness(point);
    auto fdw = pdw.fitness(point);
    auto fdtch = pdtch.fitness(point);
    auto fdbi = pdbi.fitness(point);

    EXPECT_NEAR(fdw[0], f[0] * lambda[0] + f[1] * lambda[1], 1e-8);
    EXPECT_NEAR(fdtch[0], std::max(lambda[0] * std::abs(f[0] - z[0]), lambda[1] * std::abs(f[1] - z[1])), 1e-8);
    double lnorm = std::sqrt(lambda[0] * lambda[0] + lambda[1] * lambda[1]);
    vector_double ilambda{lambda[0] / lnorm, lambda[1] / lnorm};
    double d1 = (f[0] - z[0]) * ilambda[0] + (f[1] - z[1]) * ilambda[1];
    double d20 = f[0] - (z[0] + d1 * ilambda[0]);
    double d21 = f[1] - (z[1] + d1 * ilambda[1]);
    d20 *= d20;
    d21 *= d21;
    double d2 = std::sqrt(d20 + d21);
    EXPECT_NEAR(fdbi[0], d1 + 5.0 * d2, 1e-8);
}

TEST(decompose_test, original_fitness_test)
{
    zdt p{zdt{1u, 2u}};
    vector_double lambda{0.5, 0.5};
    vector_double z{0., 0.};
    decompose pdw{zdt{1u, 2u}, lambda, z, "weighted", false};
    decompose pdtch{zdt{1u, 2u}, lambda, z, "tchebycheff", false};
    decompose pdbi{zdt{1u, 2u}, lambda, z, "bi", false};

    vector_double dv{1., 1.};
    auto f = p.fitness(dv);
    auto fdw = pdw.original_fitness(dv);
    auto fdtch = pdtch.original_fitness(dv);
    auto fdbi = pdbi.original_fitness(dv);

    // We check that the original fitness is always the same
    EXPECT_TRUE(f == fdw);
    EXPECT_TRUE(f == fdtch);
    EXPECT_TRUE(f == fdbi);
}

TEST(decompose_test, decompose_ideal_point_adaptation_test)
{
    // no adaptation
    {
        problem p{decompose{zdt{1u, 2u}, {0.5, 0.5}, {2., 2.}, "weighted", false}};
        EXPECT_TRUE(p.extract<decompose>()->get_z() == vector_double({2., 2.}));
        p.fitness({1., 1.});
        EXPECT_TRUE(p.extract<decompose>()->get_z() == vector_double({2., 2.}));
    }

    // adaptation at work
    {
        problem p{decompose{zdt{1u, 2u}, {0.5, 0.5}, {2., 2.}, "weighted", true}};
        EXPECT_TRUE(p.extract<decompose>()->get_z() == vector_double({2., 2.}));
        p.fitness({1., 1.});
        EXPECT_TRUE(p.extract<decompose>()->get_z() == vector_double({1., 2.}));
        p.fitness({0., 0.});
        EXPECT_TRUE(p.extract<decompose>()->get_z() == vector_double({0., 1.}));
    }
}

TEST(decompose_test, decompose_has_dense_sparsities_test)
{
    problem p{decompose{zdt{1u, 2u}, {0.5, 0.5}, {2., 2.}, "weighted", false}};
    EXPECT_TRUE(p.gradient_sparsity() == detail::dense_gradient(1u, 2u));
    EXPECT_TRUE(p.hessians_sparsity() == detail::dense_hessians(1u, 2u));
}

TEST(decompose_test, decompose_name_and_extra_info_test)
{
    decompose p{zdt{1u, 2u}, {0.5, 0.5}, {2., 2.}, "weighted", false};
    EXPECT_TRUE(p.get_name().find("[decomposed]") != std::string::npos);
    EXPECT_TRUE(p.get_extra_info().find("Ideal point adaptation") != std::string::npos);
}

TEST(decompose_test, decompose_serialization_test)
{
    problem p{decompose{zdt{1u, 2u}, {0.5, 0.5}, {2., 2.}, "weighted", false}};
    // Call objfun to increase the internal counters.
    p.fitness({1., 1.});
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

template <typename T>
void check_inheritance(T udp, const vector_double &w, const vector_double &r)
{
    EXPECT_EQ(problem(decompose(udp, w, r)).get_nobj(), 1u);
    EXPECT_EQ(problem(decompose(udp, w, r)).get_nix(), problem(udp).get_nix());
    EXPECT_TRUE(problem(decompose(udp, w, r)).get_bounds() == problem(udp).get_bounds());
    EXPECT_EQ(problem(decompose(udp, w, r)).has_set_seed(), problem(udp).has_set_seed());
}

struct smobjp {
    smobjp(unsigned seed = 0u) : m_seed(seed) {}
    vector_double fitness(const vector_double &) const
    {
        return {1u, 1u};
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{0.}, {1.}};
    }
    vector_double::size_type get_nobj() const
    {
        return 2u;
    }
    void set_seed(unsigned seed)
    {
        m_seed = seed;
    }
    std::string get_extra_info() const
    {
        return "Seed: " + std::to_string(m_seed);
    }
    unsigned m_seed;
};

TEST(decompose_test, decompose_inheritance_test)
{
    check_inheritance(zdt{1u, 2u}, vector_double{0.5, 0.5}, vector_double{1.5, 1.5});
    // We check the forwarding of the integer dimension
    check_inheritance(null_problem{2u, 0u, 0u, 1u}, vector_double{0.5, 0.5}, vector_double{1.5, 1.5});
    check_inheritance(null_problem{2u, 0u, 0u, 0u}, vector_double{0.5, 0.5}, vector_double{1.5, 1.5});

    // We check set_seed is working
    problem p{decompose{smobjp(1234567u), vector_double{0.5, 0.5}, vector_double{1.5, 1.5}}};
    std::ostringstream ss1, ss2;
    ss1 << p;
    EXPECT_TRUE(ss1.str().find(std::to_string(1234567u)) != std::string::npos);
    p.set_seed(5672543u);
    ss2 << p;
    EXPECT_TRUE(ss2.str().find(std::to_string(5672543u)) != std::string::npos);
}

TEST(decompose_test, decompose_inner_algo_get_test)
{
    // We check that the correct overload is called according to (*this) being const or not
    {
        const decompose udp(zdt{1u, 2u}, {0.5, 0.5}, {2., 2.}, "weighted", false);
        EXPECT_TRUE(std::is_const<decltype(udp)>::value);
        EXPECT_TRUE(std::is_const<std::remove_reference<decltype(udp.get_inner_problem())>::type>::value);
    }
    {
        decompose udp(zdt{1u, 2u}, {0.5, 0.5}, {2., 2.}, "weighted", false);
        EXPECT_TRUE(!std::is_const<decltype(udp)>::value);
        EXPECT_TRUE(!std::is_const<std::remove_reference<decltype(udp.get_inner_problem())>::type>::value);
    }
}

struct ts2 {
    vector_double fitness(const vector_double &) const
    {
        return {2, 2, 2};
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{0}, {1}};
    }
    vector_double::size_type get_nobj() const
    {
        return 2u;
    }
    thread_safety get_thread_safety() const
    {
        return thread_safety::none;
    }
};

TEST(decompose_test, decompose_thread_safety_test)
{
    zdt p0{1, 2};
    decompose t{p0, {0.5, 0.5}, {2., 2.}};
    EXPECT_TRUE(t.get_thread_safety() == thread_safety::basic);
    EXPECT_TRUE((decompose{ts2{}, {0.5, 0.5}, {2., 2.}}.get_thread_safety() == thread_safety::none));
}
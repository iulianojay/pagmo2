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
#include <limits>
#include <stdexcept>
#include <string>

#include <pagmo/exceptions.hpp>
#include <pagmo/io.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/problems/cec2006.hpp>
#include <pagmo/problems/cec2009.hpp>
#include <pagmo/problems/null_problem.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/problems/unconstrain.hpp>
#include <pagmo/problems/zdt.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>

using namespace pagmo;
TEST(unconstrain_test, unconstrain_construction_test)
{
    null_problem constrained_udp{2, 3, 4};
    // We test the default constructor
    EXPECT_NO_THROW(unconstrain{});
    EXPECT_NO_THROW(problem{unconstrain{}});
    // We test the constructor
    EXPECT_NO_THROW((problem{unconstrain{constrained_udp, "death penalty"}}));
    EXPECT_NO_THROW((problem{unconstrain{constrained_udp, "kuri"}}));
    EXPECT_NO_THROW((problem{unconstrain{constrained_udp, "weighted", vector_double(7, 1.)}}));
    EXPECT_NO_THROW((problem{unconstrain{constrained_udp, "ignore_c"}}));
    EXPECT_NO_THROW((problem{unconstrain{constrained_udp, "ignore_o"}}));

    EXPECT_EQ((problem{unconstrain{constrained_udp, "death penalty"}}.get_nc()), 0u);
    EXPECT_EQ((problem{unconstrain{constrained_udp, "kuri"}}.get_nc()), 0u);
    EXPECT_EQ((problem{unconstrain{constrained_udp, "weighted", vector_double(7, 1.)}}.get_nc()), 0u);
    EXPECT_EQ((problem{unconstrain{constrained_udp, "ignore_c"}}.get_nc()), 0u);
    EXPECT_EQ((problem{unconstrain{constrained_udp, "ignore_o"}}.get_nc()), 0u);

    EXPECT_EQ((problem{unconstrain{constrained_udp, "death penalty"}}.get_nobj()), 2u);
    EXPECT_EQ((problem{unconstrain{constrained_udp, "kuri"}}.get_nobj()), 2u);
    EXPECT_EQ((problem{unconstrain{constrained_udp, "weighted", vector_double(7, 1.)}}.get_nobj()), 2u);
    EXPECT_EQ((problem{unconstrain{constrained_udp, "ignore_c"}}.get_nobj()), 2u);
    EXPECT_EQ((problem{unconstrain{constrained_udp, "ignore_o"}}.get_nobj()), 1u);

    EXPECT_EQ((problem{unconstrain{constrained_udp, "death penalty"}}.has_gradient()), false);
    EXPECT_EQ((problem{unconstrain{constrained_udp, "death penalty"}}.has_hessians()), false);
    // We test the various throws
    EXPECT_THROW((unconstrain{null_problem{2, 0, 0}, "kuri"}), incompatible_problem_error);
    EXPECT_THROW((unconstrain{null_problem{2, 3, 4}, "weighted", vector_double(6, 1.)}), dimension_mismatch_error);
    EXPECT_THROW((unconstrain{null_problem{2, 3, 4}, "mispelled"}), invalid_parameter_error);
    EXPECT_THROW((unconstrain{null_problem{2, 3, 4}, "kuri", vector_double(3, 1.)}), invalid_parameter_error);
}

struct my_udp {
    vector_double fitness(const vector_double &x) const
    {
        return x;
    }
    vector_double::size_type get_nobj() const
    {
        return 2u;
    }
    vector_double::size_type get_nec() const
    {
        return 2u;
    }
    vector_double::size_type get_nic() const
    {
        return 2u;
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{-1, -1, -1, -1, -1, -1}, {1, 1, 1, 1, 1, 1}};
    }
    std::string get_name() const
    {
        return "a simple problem with constraint for testing";
    }
    template <typename Archive>
    void serialize(Archive &, unsigned)
    {
    }
};

PAGMO_S11N_PROBLEM_EXPORT(my_udp)

TEST(unconstrain_test, unconstrain_fitness_test)
{
    {
        unconstrain p0{my_udp{}, "death penalty"};
        // we test the method death penalty
        EXPECT_TRUE(p0.fitness(vector_double(6, 0.)) == vector_double(2, 0.));
        EXPECT_TRUE(p0.fitness(vector_double(6, 1.)) == vector_double(2, std::numeric_limits<double>::max()));
    }
    {
        unconstrain p0{my_udp{}, "kuri"};
        // we test the method kuri
        EXPECT_TRUE(p0.fitness(vector_double(6, 0.)) == vector_double(2, 0.));
        EXPECT_TRUE(p0.fitness(vector_double{0., 0., 1., 1., -1., 1.})
                    == vector_double(2, std::numeric_limits<double>::max() * (1. - 1. / 4.)));
        EXPECT_TRUE(p0.fitness(vector_double{0., 0., 1., 1., -1., -1.})
                    == vector_double(2, std::numeric_limits<double>::max() * (1. - 2. / 4.)));
        EXPECT_TRUE(p0.fitness(vector_double{0., 0., 0., 1., 0., 0.})
                    == vector_double(2, std::numeric_limits<double>::max() * (1. - 3. / 4.)));
    }
    {
        unconstrain p0{my_udp{}, "weighted", vector_double(4, 1.)};
        EXPECT_TRUE(p0.fitness(vector_double(6, 0.)) == vector_double(2, 0.));
        EXPECT_TRUE(p0.fitness(vector_double{0., 0., 1., 1., -1., 1.}) == vector_double(2, 3.));
        EXPECT_TRUE(p0.fitness(vector_double{0., 0., 1., 1., -1., -1.}) == vector_double(2, 2.));
        EXPECT_TRUE(p0.fitness(vector_double{0., 0., 0., 1., 0., 0.}) == vector_double(2, 1.));
        vector_double nan_fitness
            = p0.fitness(vector_double{0., 0., std::numeric_limits<double>::quiet_NaN(), 1., -1., 1.});
        EXPECT_TRUE(std::isnan(nan_fitness[0]));
        EXPECT_TRUE(std::isnan(nan_fitness[1]));
    }
    {
        unconstrain p0{my_udp{}, "ignore_c"};
        EXPECT_TRUE(p0.fitness(vector_double(6, 0.)) == vector_double(2, 0.));
        EXPECT_TRUE((p0.fitness(vector_double{1., 2., 1., 1., -1., 1.}) == vector_double{1., 2.}));
        EXPECT_TRUE((p0.fitness(vector_double{3., 4., 1., 1., -1., -1.}) == vector_double{3., 4.}));
        EXPECT_TRUE((p0.fitness(vector_double{5., 6., 0., 1., 0., 0.}) == vector_double{5., 6.}));
    }
    {
        unconstrain p0{my_udp{}, "ignore_o"};
        EXPECT_TRUE(p0.fitness(vector_double(6, 0.)) == vector_double(1, 0.));
        EXPECT_TRUE((p0.fitness(vector_double{1., 2., 1., 0., -1., -1.}) == vector_double{1.}));
        EXPECT_NEAR(p0.fitness(vector_double{1., 2., 1., 1., -1., -1.})[0], std::sqrt(2.), 1e-8);
        EXPECT_NEAR(p0.fitness(vector_double{1., 2., 1., 1., 1., -1.})[0], std::sqrt(2.) + std::sqrt(1.), 1e-8);
        EXPECT_NEAR(p0.fitness(vector_double{1., 2., 1., 1., 1., 2.})[0], std::sqrt(2.) + std::sqrt(5.), 1e-8);
    }
}

TEST(unconstrain_test, unconstrain_various_test)
{
    unconstrain p0{my_udp{}, "death penalty"};
    unconstrain p1{my_udp{}, "weighted", vector_double(4, 1.)};
    EXPECT_EQ(p0.get_nobj(), 2u);
    EXPECT_TRUE(p0.get_name().find("[unconstrained]") != std::string::npos);
    EXPECT_TRUE(p0.get_extra_info().find("Weight vector") == std::string::npos);
    EXPECT_TRUE(p0.get_extra_info().find("death penalty") != std::string::npos);
    EXPECT_TRUE(p1.get_extra_info().find("Weight vector") != std::string::npos);
    EXPECT_TRUE(p1.get_extra_info().find("weighted") != std::string::npos);
}

TEST(unconstrain_test, unconstrain_serialization_test)
{
    problem p{unconstrain{my_udp{}, "kuri"}};
    // Call objfun to increase the internal counters.
    p.fitness({1., 1., 1., 1., 1., 1.});
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
void check_inheritance(T udp)
{
    EXPECT_EQ(problem(unconstrain(udp)).get_nobj(), problem(udp).get_nobj());
    EXPECT_EQ(problem(unconstrain(udp)).get_nc(), 0u);
    EXPECT_TRUE(problem(unconstrain(udp)).get_bounds() == problem(udp).get_bounds());
    EXPECT_EQ(problem(unconstrain(udp)).has_set_seed(), problem(udp).has_set_seed());
    EXPECT_EQ(problem(unconstrain(udp)).get_nix(), problem(udp).get_nix());
}

struct sconp {
    sconp(unsigned seed = 0u) : m_seed(seed) {}
    vector_double fitness(const vector_double &) const
    {
        return {1u, 1u, 1u};
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{0.}, {1.}};
    }
    vector_double::size_type get_nec() const
    {
        return 1u;
    }
    vector_double::size_type get_nic() const
    {
        return 1u;
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

TEST(unconstrain_test, unconstrain_inheritance_test)
{
    check_inheritance(my_udp{});
    check_inheritance(null_problem{2, 3, 4});
    check_inheritance(null_problem{2, 3, 4, 1});
    check_inheritance(null_problem{2, 3, 4, 0});
    check_inheritance(cec2006{2});
    check_inheritance(cec2009{4, true});
    // We check set_seed is working
    problem p{unconstrain{sconp(1234567u)}};
    std::ostringstream ss1, ss2;
    ss1 << p;
    EXPECT_TRUE(ss1.str().find(std::to_string(1234567u)) != std::string::npos);
    p.set_seed(5672543u);
    ss2 << p;
    EXPECT_TRUE(ss2.str().find(std::to_string(5672543u)) != std::string::npos);
}

TEST(unconstrain_test, unconstrain_inner_algo_get_test)
{
    // We check that the correct overload is called according to (*this) being const or not
    {
        const unconstrain udp(null_problem{2, 2, 2});
        EXPECT_TRUE(std::is_const<decltype(udp)>::value);
        EXPECT_TRUE(std::is_const<std::remove_reference<decltype(udp.get_inner_problem())>::type>::value);
    }
    {
        unconstrain udp(null_problem{2, 2, 2});
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
    vector_double::size_type get_nic() const
    {
        return 1u;
    }
    thread_safety get_thread_safety() const
    {
        return thread_safety::none;
    }
};

TEST(unconstrain_test, unconstrain_thread_safety_test)
{
    null_problem p0{2, 2, 3};
    unconstrain t{p0};
    EXPECT_TRUE(t.get_thread_safety() == thread_safety::basic);
    EXPECT_TRUE((unconstrain{ts2{}}.get_thread_safety() == thread_safety::none));
}

// UDP which implements batch_fitness.
struct bf0 {
    vector_double fitness(const vector_double &) const
    {
        return {0, 0};
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{0}, {1}};
    }
    vector_double batch_fitness(const vector_double &dvs) const
    {
        ++s_counter;
        return vector_double(dvs.size() * 2u, 1.);
    }
    vector_double::size_type get_nic() const
    {
        return 1u;
    }
    static unsigned s_counter;
};
unsigned bf0::s_counter = 0;

TEST(unconstrain_test, unconstrain_batch_test)
{
    problem p0{bf0{}};
    unconstrain t{p0};
    EXPECT_TRUE(t.has_batch_fitness());
    bfe default_bfe{};
    population pop{p0, default_bfe, 20u};
    EXPECT_TRUE(bf0::s_counter == 1u);

    // through batch_fitness
    vector_double::size_type bs = 10;
    vector_double xs(p0.get_nx() * bs, 1.);
    vector_double ys = t.batch_fitness(xs);
    EXPECT_TRUE(ys.size() == t.get_nobj() * bs);
    EXPECT_TRUE(bf0::s_counter == 2u);
}

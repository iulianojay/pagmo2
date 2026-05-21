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

#include <any>
#include <cmath>
#include <gtest/gtest.h>
#include <initializer_list>
#include <limits>
#include <nlopt.h>
#include <pagmo/utils/cast.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/nlopt.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/problems/hock_schittkowski_71.hpp>
#include <pagmo/problems/luksan_vlcek1.hpp>
#include <pagmo/problems/null_problem.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/problems/zdt.hpp>
#include <pagmo/rng.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/exceptions.hpp>

using namespace pagmo;

using hs71 = hock_schittkowski_71;

TEST(nlopt_test, nlopt_construction)
{
    random_device::set_seed(42);

    algorithm a{nlopt{}};
    EXPECT_EQ(a.extract<nlopt>()->get_solver_name(), "cobyla");
    // Check params of default-constructed instance.
    EXPECT_EQ(std::any_cast<std::string>(a.extract<nlopt>()->get_selection()), "best");
    EXPECT_EQ(std::any_cast<std::string>(a.extract<nlopt>()->get_replacement()), "best");
    EXPECT_TRUE(a.extract<nlopt>()->get_name() != "");
    EXPECT_TRUE(a.extract<nlopt>()->get_extra_info() != "");
    EXPECT_TRUE(a.extract<nlopt>()->get_last_opt_result() == NLOPT_SUCCESS);
    EXPECT_EQ(a.extract<nlopt>()->get_stopval(), -HUGE_VAL);
    EXPECT_EQ(a.extract<nlopt>()->get_ftol_rel(), 0.);
    EXPECT_EQ(a.extract<nlopt>()->get_ftol_abs(), 0.);
    EXPECT_EQ(a.extract<nlopt>()->get_xtol_rel(), 1E-8);
    EXPECT_EQ(a.extract<nlopt>()->get_xtol_abs(), 0.);
    EXPECT_EQ(a.extract<nlopt>()->get_maxeval(), 0);
    EXPECT_EQ(a.extract<nlopt>()->get_maxtime(), 0);
    // Change a few params and copy.
    a.extract<nlopt>()->set_selection(12u);
    a.extract<nlopt>()->set_replacement("random");
    a.extract<nlopt>()->set_ftol_abs(1E-5);
    a.extract<nlopt>()->set_maxeval(123);
    // Copy.
    auto b(a);
    EXPECT_EQ(std::any_cast<population::size_type>(b.extract<nlopt>()->get_selection()), 12u);
    EXPECT_EQ(std::any_cast<std::string>(b.extract<nlopt>()->get_replacement()), "random");
    EXPECT_TRUE(b.extract<nlopt>()->get_last_opt_result() == NLOPT_SUCCESS);
    EXPECT_EQ(b.extract<nlopt>()->get_stopval(), -HUGE_VAL);
    EXPECT_EQ(b.extract<nlopt>()->get_ftol_rel(), 0.);
    EXPECT_EQ(b.extract<nlopt>()->get_ftol_abs(), 1E-5);
    EXPECT_EQ(b.extract<nlopt>()->get_xtol_rel(), 1E-8);
    EXPECT_EQ(b.extract<nlopt>()->get_xtol_abs(), 0.);
    EXPECT_EQ(b.extract<nlopt>()->get_maxeval(), 123);
    EXPECT_EQ(b.extract<nlopt>()->get_maxtime(), 0);
    algorithm c;
    c = b;
    EXPECT_EQ(std::any_cast<population::size_type>(c.extract<nlopt>()->get_selection()), 12u);
    EXPECT_EQ(std::any_cast<std::string>(c.extract<nlopt>()->get_replacement()), "random");
    EXPECT_TRUE(c.extract<nlopt>()->get_last_opt_result() == NLOPT_SUCCESS);
    EXPECT_EQ(c.extract<nlopt>()->get_stopval(), -HUGE_VAL);
    EXPECT_EQ(c.extract<nlopt>()->get_ftol_rel(), 0.);
    EXPECT_EQ(c.extract<nlopt>()->get_ftol_abs(), 1E-5);
    EXPECT_EQ(c.extract<nlopt>()->get_xtol_rel(), 1E-8);
    EXPECT_EQ(c.extract<nlopt>()->get_xtol_abs(), 0.);
    EXPECT_EQ(c.extract<nlopt>()->get_maxeval(), 123);
    EXPECT_EQ(c.extract<nlopt>()->get_maxtime(), 0);
    // Move.
    auto tmp(*a.extract<nlopt>());
    auto d(std::move(tmp));
    EXPECT_EQ(std::any_cast<population::size_type>(d.get_selection()), 12u);
    EXPECT_EQ(std::any_cast<std::string>(d.get_replacement()), "random");
    EXPECT_TRUE(d.get_last_opt_result() == NLOPT_SUCCESS);
    EXPECT_EQ(d.get_stopval(), -HUGE_VAL);
    EXPECT_EQ(d.get_ftol_rel(), 0.);
    EXPECT_EQ(d.get_ftol_abs(), 1E-5);
    EXPECT_EQ(d.get_xtol_rel(), 1E-8);
    EXPECT_EQ(d.get_xtol_abs(), 0.);
    EXPECT_EQ(d.get_maxeval(), 123);
    EXPECT_EQ(d.get_maxtime(), 0);
    nlopt e;
    e = std::move(d);
    EXPECT_EQ(std::any_cast<population::size_type>(e.get_selection()), 12u);
    EXPECT_EQ(std::any_cast<std::string>(e.get_replacement()), "random");
    EXPECT_TRUE(e.get_last_opt_result() == NLOPT_SUCCESS);
    EXPECT_EQ(e.get_stopval(), -HUGE_VAL);
    EXPECT_EQ(e.get_ftol_rel(), 0.);
    EXPECT_EQ(e.get_ftol_abs(), 1E-5);
    EXPECT_EQ(e.get_xtol_rel(), 1E-8);
    EXPECT_EQ(e.get_xtol_abs(), 0.);
    EXPECT_EQ(e.get_maxeval(), 123);
    EXPECT_EQ(e.get_maxtime(), 0);
    // Revive moved-from.
    d = std::move(e);
    EXPECT_EQ(std::any_cast<population::size_type>(d.get_selection()), 12u);
    EXPECT_EQ(std::any_cast<std::string>(d.get_replacement()), "random");
    EXPECT_TRUE(d.get_last_opt_result() == NLOPT_SUCCESS);
    EXPECT_EQ(d.get_stopval(), -HUGE_VAL);
    EXPECT_EQ(d.get_ftol_rel(), 0.);
    EXPECT_EQ(d.get_ftol_abs(), 1E-5);
    EXPECT_EQ(d.get_xtol_rel(), 1E-8);
    EXPECT_EQ(d.get_xtol_abs(), 0.);
    EXPECT_EQ(d.get_maxeval(), 123);
    EXPECT_EQ(d.get_maxtime(), 0);
    // Check exception throwing on ctor.
    EXPECT_THROW(nlopt{""}, pagmo_exception);
}

TEST(nlopt_test, nlopt_selection_replacement)
{
    nlopt a;
    a.set_selection("worst");
    EXPECT_EQ(std::any_cast<std::string>(a.get_selection()), "worst");
    EXPECT_THROW(a.set_selection("worstee"), pagmo_exception);
    a.set_selection(0);
    EXPECT_EQ(std::any_cast<population::size_type>(a.get_selection()), 0u);
    a.set_replacement("worst");
    EXPECT_EQ(std::any_cast<std::string>(a.get_replacement()), "worst");
    EXPECT_THROW(a.set_replacement("worstee"), pagmo_exception);
    a.set_replacement(0);
    EXPECT_EQ(std::any_cast<population::size_type>(a.get_replacement()), 0u);
    a.set_random_sr_seed(123);
}

// A version of hs71 which provides the sparsity pattern.
struct hs71a : hs71 {
    sparsity_pattern gradient_sparsity() const
    {
        return detail::dense_gradient(3, 4);
    }
};

TEST(nlopt_test, nlopt_evolve)
{
    algorithm a{nlopt{"lbfgs"}};
    population pop(rosenbrock{10}, 20);
    a.evolve(pop);
    EXPECT_TRUE(a.extract<nlopt>()->get_last_opt_result() >= 0);
    pop = population{zdt{}, 20};
    // MOO not supported by NLopt.
    EXPECT_THROW(a.evolve(pop), pagmo_exception);
    // Solver wants gradient, but problem does not provide it.
    pop = population{null_problem{}, 20};
    EXPECT_THROW(a.evolve(pop), pagmo_exception);
    pop = population{hs71{}, 20};
    // lbfgs does not support ineq constraints.
    EXPECT_THROW(a.evolve(pop), pagmo_exception);
    // mma supports ineq constraints but not eq constraints.
    EXPECT_THROW(algorithm{nlopt{"mma"}}.evolve(pop), pagmo_exception);
    a = algorithm{nlopt{"slsqp"}};
    a.extract<nlopt>()->set_verbosity(5);
    for (auto s : {"best", "worst", "random"}) {
        for (auto r : {"best", "worst", "random"}) {
            a.extract<nlopt>()->set_selection(s);
            a.extract<nlopt>()->set_replacement(r);
            pop = population(rosenbrock{10}, 20);
            a.evolve(pop);
            pop = population{hs71{}, 20};
            pop.get_problem().set_c_tol({1E-6, 1E-6});
            a.evolve(pop);
            pop = population{hs71a{}, 20};
            pop.get_problem().set_c_tol({1E-6, 1E-6});
            a.evolve(pop);
        }
    }
    EXPECT_TRUE(!a.extract<nlopt>()->get_log().empty());
    for (auto s : {0u, 2u, 15u, 25u}) {
        for (auto r : {1u, 3u, 16u, 25u}) {
            a.extract<nlopt>()->set_selection(s);
            a.extract<nlopt>()->set_replacement(r);
            pop = population(rosenbrock{10}, 20);
            if (s >= 20u || r >= 20u) {
                EXPECT_THROW(a.evolve(pop), pagmo_exception);
                continue;
            }
            a.evolve(pop);
            pop = population{hs71{}, 20};
            pop.get_problem().set_c_tol({1E-6, 1E-6});
            a.evolve(pop);
            pop = population{hs71a{}, 20};
            pop.get_problem().set_c_tol({1E-6, 1E-6});
            a.evolve(pop);
        }
    }
    // Empty evolve.
    a.evolve(population{});
    // Invalid initial guesses.
    a = algorithm{nlopt{"slsqp"}};
    pop = population{hs71{}, 1};
    pop.set_x(0, {-123., -123., -123., -123.});
    EXPECT_THROW(a.evolve(pop), pagmo_exception);
    pop.set_x(0, {123., 123., 123., 123.});
    EXPECT_THROW(a.evolve(pop), pagmo_exception);
    if (std::numeric_limits<double>::has_quiet_NaN) {
        pop.set_x(0, {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(),
                      std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()});
        EXPECT_THROW(a.evolve(pop), pagmo_exception);
    }
}

TEST(nlopt_test, nlopt_set_sc)
{
    auto a = nlopt{"slsqp"};
    a.set_stopval(-1.23);
    EXPECT_EQ(a.get_stopval(), -1.23);
    if (std::numeric_limits<double>::has_quiet_NaN) {
        EXPECT_THROW(a.set_stopval(std::numeric_limits<double>::quiet_NaN()), pagmo_exception);
    }
    a.set_ftol_rel(-1.23);
    EXPECT_EQ(a.get_ftol_rel(), -1.23);
    if (std::numeric_limits<double>::has_quiet_NaN) {
        EXPECT_THROW(a.set_ftol_rel(std::numeric_limits<double>::quiet_NaN()), pagmo_exception);
    }
    a.set_ftol_abs(-1.23);
    EXPECT_EQ(a.get_ftol_abs(), -1.23);
    if (std::numeric_limits<double>::has_quiet_NaN) {
        EXPECT_THROW(a.set_ftol_abs(std::numeric_limits<double>::quiet_NaN()), pagmo_exception);
    }
    a.set_xtol_rel(-1.23);
    EXPECT_EQ(a.get_xtol_rel(), -1.23);
    if (std::numeric_limits<double>::has_quiet_NaN) {
        EXPECT_THROW(a.set_xtol_rel(std::numeric_limits<double>::quiet_NaN()), pagmo_exception);
    }
    a.set_xtol_abs(-1.23);
    EXPECT_EQ(a.get_xtol_abs(), -1.23);
    if (std::numeric_limits<double>::has_quiet_NaN) {
        EXPECT_THROW(a.set_xtol_abs(std::numeric_limits<double>::quiet_NaN()), pagmo_exception);
    }
    a.set_maxtime(123);
}

TEST(nlopt_test, nlopt_serialization)
{
    for (auto r : {"best", "worst", "random"}) {
        for (auto s : {"best", "worst", "random"}) {
            auto n = nlopt{"slsqp"};
            n.set_replacement(r);
            n.set_selection(s);
            algorithm algo{n};
            algo.set_verbosity(5);
            auto pop = population(hs71{}, 10);
            algo.evolve(pop);
            auto s_log = algo.extract<nlopt>()->get_log();
            // Store the string representation of p.
            std::stringstream ss;
            auto before_text = lexical_cast<std::string>(algo);
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
            EXPECT_EQ(before_text, after_text);
            EXPECT_TRUE(s_log == algo.extract<nlopt>()->get_log());
        }
    }
    for (auto r : {0u, 4u, 7u}) {
        for (auto s : {0u, 4u, 7u}) {
            auto n = nlopt{"slsqp"};
            n.set_replacement(r);
            n.set_selection(s);
            algorithm algo{n};
            algo.set_verbosity(5);
            auto pop = population(hs71{}, 10);
            algo.evolve(pop);
            auto s_log = algo.extract<nlopt>()->get_log();
            // Store the string representation of p.
            std::stringstream ss;
            auto before_text = lexical_cast<std::string>(algo);
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
            EXPECT_EQ(before_text, after_text);
            EXPECT_TRUE(s_log == algo.extract<nlopt>()->get_log());
        }
    }
}

TEST(nlopt_test, nlopt_loc_opt)
{
    for (const auto &str : {"auglag", "auglag_eq"}) {
        nlopt n{str};
        n.set_local_optimizer(nlopt{"slsqp"});
        EXPECT_TRUE(n.get_local_optimizer());
        EXPECT_TRUE(static_cast<const nlopt &>(n).get_local_optimizer());
        // Test serialization.
        algorithm algo{n};
        std::stringstream ss;
        auto before_text = lexical_cast<std::string>(algo);
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
        EXPECT_EQ(before_text, after_text);
        // Test small evolution.
        auto pop = population{hs71{}, 1};
        pop.set_x(0, {2., 2., 2., 2.});
        pop.get_problem().set_c_tol({1E-6, 1E-6});
        algo.evolve(pop);
        EXPECT_TRUE(algo.extract<nlopt>()->get_last_opt_result() >= 0);
        // Unset the local optimizer.
        algo.extract<nlopt>()->unset_local_optimizer();
        EXPECT_TRUE(!algo.extract<nlopt>()->get_local_optimizer());
        algo.evolve(pop);
        EXPECT_TRUE(algo.extract<nlopt>()->get_last_opt_result() == NLOPT_INVALID_ARGS);
        // Auglag inside auglag. Not sure if this is supposed to work, it gives an error
        // currently.
        algo.extract<nlopt>()->set_local_optimizer(nlopt{str});
        algo.extract<nlopt>()->get_local_optimizer()->set_local_optimizer(nlopt{"lbfgs"});
        algo.evolve(pop);
        EXPECT_TRUE(algo.extract<nlopt>()->get_last_opt_result() < 0);
    }
    // Check setting a local opt does not do anything for normal solvers.
    nlopt n{"slsqp"};
    n.set_local_optimizer(nlopt{"lbfgs"});
    algorithm algo{n};
    auto pop = population{rosenbrock{20}, 1};
    algo.evolve(pop);
    EXPECT_TRUE(algo.extract<nlopt>()->get_last_opt_result() >= 0);
}

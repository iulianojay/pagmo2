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

#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>

#include <pagmo/exceptions.hpp>
#include <pagmo/io.hpp>
#include <pagmo/problems/cec2006.hpp>
#include <pagmo/problems/cec2009.hpp>
#include <pagmo/problems/hock_schittkowski_71.hpp>
#include <pagmo/problems/inventory.hpp>
#include <pagmo/problems/null_problem.hpp>
#include <pagmo/problems/translate.hpp>
#include <pagmo/threading.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>

using namespace pagmo;

TEST(translate_test, translate_construction_test)
{
    // First we check directly the two constructors
    problem p0{translate{}};
    problem p1{translate{null_problem{}, {0.}}};

    auto p0_string = lexical_cast<std::string>(p0);
    auto p1_string = lexical_cast<std::string>(p1);

    // We check that the default constructor constructs a problem
    // which has an identical representation to the problem
    // built by the explicit constructor.
    EXPECT_TRUE(p0_string == p1_string);

    // We check that wrong size for translation results in an invalid_argument
    // exception
    EXPECT_THROW((translate{null_problem{}, {1, 2}}), dimension_mismatch_error);
}

TEST(translate_test, translate_functional_test)
{
    // Then we check that the hock_schittkowski_71 problem is actually translated
    {
        hock_schittkowski_71 hs;
        problem p0{hs};
        translate t1{hs, {0.1, -0.2, 0.3, 0.4}};
        problem p1{t1};
        problem p2{translate{t1, {-0.1, 0.2, -0.3, -0.4}}};
        vector_double x{3., 3., 3., 3.};
        // Fitness gradients and hessians are the same if the translation  is zero
        EXPECT_TRUE(p0.fitness(x) == p2.fitness(x));
        EXPECT_TRUE(p0.gradient(x) == p2.gradient(x));
        EXPECT_TRUE(p0.hessians(x) == p2.hessians(x));
        // Bounds are unchanged if the translation is zero
        EXPECT_TRUE(p0.get_bounds().first != p1.get_bounds().first);
        EXPECT_TRUE(p0.get_bounds().first != p1.get_bounds().second);
        auto bounds0 = p0.get_bounds();
        auto bounds2 = p2.get_bounds();
        for (auto i = 0u; i < 4u; ++i) {
            EXPECT_NEAR(bounds0.first[i], bounds2.first[i], 1e-13);
            EXPECT_NEAR(bounds0.second[i], bounds2.second[i], 1e-13);
        }
        // We check that the problem's name has [translated] appended
        EXPECT_TRUE(p1.get_name().find("[translated]") != std::string::npos);
        // We check that extra info has "Translation Vector:" somewhere"
        EXPECT_TRUE(p1.get_extra_info().find("Translation Vector:") != std::string::npos);
        // We check we recover the translation vector
        auto translationvector = p1.extract<translate>()->get_translation();
        EXPECT_TRUE((translationvector == vector_double{0.1, -0.2, 0.3, 0.4}));
    }
}

TEST(translate_test, translate_serialization_test)
{
    // Do the checking with the full problem.
    hock_schittkowski_71 p0{};
    problem p{translate{p0, {0.1, -0.2, 0.3, 0.4}}};
    // Call objfun, grad and hess to increase
    // the internal counters.
    p.fitness({1., 1., 1., 1.});
    p.gradient({1., 1., 1., 1.});
    p.hessians({1., 1., 1., 1.});
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

TEST(translate_test, translate_stochastic_test)
{
    hock_schittkowski_71 p0{};
    problem p{translate{p0, {0.1, -0.2, 0.3, 0.4}}};
    EXPECT_TRUE(!p.is_stochastic());
}

struct ts2 {
    vector_double fitness(const vector_double &) const
    {
        return {2};
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{0}, {1}};
    }
    thread_safety get_thread_safety() const
    {
        return thread_safety::none;
    }
};

TEST(translate_test, translate_thread_safety_test)
{
    hock_schittkowski_71 p0{};
    translate t{p0, {0.1, -0.2, 0.3, 0.4}};
    EXPECT_TRUE(t.get_thread_safety() == thread_safety::basic);
    EXPECT_TRUE((translate{ts2{}, {1}}.get_thread_safety() == thread_safety::none));
}

template <typename T>
void check_inheritance(T udp, const vector_double &t)
{
    EXPECT_EQ(problem(translate(udp, t)).get_nobj(), problem(udp).get_nobj());
    EXPECT_EQ(problem(translate(udp, t)).get_nec(), problem(udp).get_nec());
    EXPECT_EQ(problem(translate(udp, t)).get_nic(), problem(udp).get_nic());
    EXPECT_EQ(problem(translate(udp, t)).get_nix(), problem(udp).get_nix());
    EXPECT_EQ(problem(translate(udp, t)).has_gradient(), problem(udp).has_gradient());
    EXPECT_TRUE(translate(udp, t).gradient_sparsity() == problem(udp).gradient_sparsity());
    EXPECT_EQ(problem(translate(udp, t)).has_gradient_sparsity(), problem(udp).has_gradient_sparsity());
    EXPECT_EQ(problem(translate(udp, t)).has_hessians(), problem(udp).has_hessians());
    EXPECT_TRUE(problem(translate(udp, t)).hessians_sparsity() == problem(udp).hessians_sparsity());
    EXPECT_EQ(problem(translate(udp, t)).has_hessians_sparsity(), problem(udp).has_hessians_sparsity());
    EXPECT_EQ(problem(translate(udp, t)).has_set_seed(), problem(udp).has_set_seed());
}

TEST(translate_test, translate_inheritance_test)
{
    check_inheritance(hock_schittkowski_71{}, vector_double(4, 0.5));
    check_inheritance(cec2006{1}, vector_double(13, 0.5));
    check_inheritance(cec2009{1}, vector_double(30, 0.5));
    // We check the forwarding of the integer dimension. The translation needs to be integer too as to
    // not create a non integer bound.
    check_inheritance(null_problem{2, 2, 3, 1}, vector_double(1, 1));
    check_inheritance(null_problem{2, 2, 3, 0}, vector_double(1, 1));

    // We check if set_seed is working
    problem p{translate{inventory{10u, 10u, 1234567u}, vector_double(10, 1.)}};
    std::ostringstream ss1, ss2;
    ss1 << p;
    EXPECT_TRUE(ss1.str().find(std::to_string(1234567u)) != std::string::npos);
    p.set_seed(5672543u);
    ss2 << p;
    EXPECT_TRUE(ss2.str().find(std::to_string(5672543u)) != std::string::npos);
}

TEST(translate_test, translate_inner_algo_get_test)
{
    // We check that the correct overload is called according to (*this) being const or not
    {
        const translate udp(hock_schittkowski_71{}, vector_double(4, 0.5));
        EXPECT_TRUE(std::is_const<decltype(udp)>::value);
        EXPECT_TRUE(std::is_const<std::remove_reference<decltype(udp.get_inner_problem())>::type>::value);
    }
    {
        translate udp(hock_schittkowski_71{}, vector_double(4, 0.5));
        EXPECT_TRUE(!std::is_const<decltype(udp)>::value);
        EXPECT_TRUE(!std::is_const<std::remove_reference<decltype(udp.get_inner_problem())>::type>::value);
    }
}

struct udp_with_bfe {
    vector_double fitness(const vector_double &) const
    {
        return {2};
    }
    vector_double batch_fitness(const vector_double &xs) const
    {
        vector_double fvs(xs.size() / 2u);

        for (decltype(xs.size()) i = 0; i < xs.size() / 2u; ++i) {
            fvs[i] = 10. * (xs[i] + xs[i + 1u]);
        }

        return fvs;
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{0, 0}, {1, 1}};
    }
};

TEST(translate_test, translate_batch_fitness_test)
{
    problem p0{udp_with_bfe{}};
    problem p1{translate{udp_with_bfe{}, {1, 1}}};
    problem p2{translate{translate{udp_with_bfe{}, {1, 1}}, {-1, -1}}};

    EXPECT_TRUE(p0.has_batch_fitness());
    EXPECT_TRUE(p1.has_batch_fitness());
    EXPECT_TRUE(p2.has_batch_fitness());

    vector_double dvs(10000u * 2u);
    std::iota(dvs.begin(), dvs.end(), 0.);

    auto fvs0 = p0.batch_fitness(dvs);
    auto fvs1 = p1.batch_fitness(dvs);
    auto fvs2 = p2.batch_fitness(dvs);

    EXPECT_TRUE(fvs0 != fvs1);
    EXPECT_TRUE(fvs0 == fvs2);

    auto no_bfe = problem{translate{hock_schittkowski_71{}, {0.1, -0.2, 0.3, 0.4}}};
    EXPECT_TRUE(!no_bfe.has_batch_fitness());
    EXPECT_THROW(no_bfe.batch_fitness({3., 3., 3., 3.}), not_implemented_error);
}

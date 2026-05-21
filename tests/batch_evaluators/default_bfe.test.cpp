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

#include <initializer_list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <pagmo/batch_evaluators/default_bfe.hpp>
#include <pagmo/batch_evaluators/member_bfe.hpp>
#include <pagmo/bfe.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/threading.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>
#include <pagmo/exceptions.hpp>

using namespace pagmo;

// UDP which implements batch_fitness.
struct bf0 {
    vector_double fitness(const vector_double &) const
    {
        return {0};
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{0}, {1}};
    }
    vector_double batch_fitness(const vector_double &dvs) const
    {
        ++s_counter;
        return vector_double(dvs.size(), 1.);
    }
    static unsigned s_counter;
};

unsigned bf0::s_counter = 0;

// UDP without batch_fitness, but supporting thread_bfe.
struct bf1 {
    vector_double fitness(const vector_double &) const
    {
        return {0};
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{0}, {1}};
    }
};

// UDP without batch_fitness, and not thread-safe.
struct bf2 {
    vector_double fitness(const vector_double &) const
    {
        return {0};
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{0}, {1}};
    }
    thread_safety get_thread_safety() const
    {
        return thread_safety::none;
    }
    std::string get_name() const
    {
        return "baffo";
    }
};

TEST(default_bfe_test, basic_tests)
{
    EXPECT_TRUE(IsUdBfe<default_bfe>);

    bfe bfe0{};
    EXPECT_TRUE(bfe0.is<default_bfe>());
    EXPECT_TRUE(bfe0.get_name() == "Default batch fitness evaluator");
    EXPECT_TRUE(bfe0.get_extra_info().empty());
    EXPECT_EQ(bfe0.get_thread_safety(), thread_safety::basic);

    // Use UDP member function.
    problem p{bf0{}};
    EXPECT_TRUE(p.get_fevals() == 0u);
    EXPECT_TRUE(bfe0(p, {1., 2., 3.}) == vector_double(3, 1.));
    // Check fevals counters.
    EXPECT_TRUE(p.get_fevals() == 3u);
    EXPECT_TRUE(bf0::s_counter == 1u);

    // UDP without batch_fitness() member function, but supporting thread_bfe.
    p = problem{bf1{}};
    EXPECT_TRUE(p.get_fevals() == 0u);
    EXPECT_TRUE(bfe0(p, {1., 2., 3.}) == vector_double(3, 0.));
    // Check fevals counters.
    EXPECT_TRUE(p.get_fevals() == 3u);

    // UDP without batch_fitness() member function, which is not thread-safe enough.
    p = problem{bf2{}};
    EXPECT_TRUE(p.get_thread_safety() == thread_safety::none);
    EXPECT_THROW(bfe0(p, {1.}), batch_eval_error);
}

TEST(default_bfe_test, s11n)
{
    bfe bfe0{default_bfe{}};
    EXPECT_TRUE(bfe0.is<default_bfe>());
    // Store the string representation.
    std::stringstream ss;
    auto before = lexical_cast<std::string>(bfe0);
    // Now serialize, deserialize and compare the result.
    {
        cereal::BinaryOutputArchive oarchive(ss);
        oarchive(bfe0);
    }
    // Change the content of p before deserializing.
    bfe0 = bfe{member_bfe{}};
    EXPECT_TRUE(!bfe0.is<default_bfe>());
    {
        cereal::BinaryInputArchive iarchive(ss);
        iarchive(bfe0);
    }
    auto after = lexical_cast<std::string>(bfe0);
    EXPECT_EQ(before, after);
    EXPECT_TRUE(bfe0.is<default_bfe>());
}

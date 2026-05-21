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
#include <stdexcept>
#include <utility>

#include <pagmo/island.hpp>
#include <pagmo/islands/thread_island.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/threading.hpp>
#include <pagmo/types.hpp>
#include <pagmo/exceptions.hpp>

using namespace pagmo;

// Thread-unsafe UDA.
struct tu_uda {
    population evolve(const population &pop) const
    {
        return pop;
    }
    thread_safety get_thread_safety() const
    {
        return thread_safety::none;
    }
};

// Thread unsafe UDP.
struct tu_udp {
    vector_double fitness(const vector_double &) const
    {
        return {1};
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

TEST(thread_island_test, thread_island_test)
{
    {
        island isl(thread_island{}, tu_uda{}, problem(), 20u);
        isl.evolve();
        EXPECT_THROW(isl.wait_check(), incompatible_problem_error);
    }
    {
        island isl(thread_island{}, algorithm(), tu_udp{}, 20u);
        isl.evolve();
        EXPECT_THROW(isl.wait_check(), incompatible_problem_error);
    }
}

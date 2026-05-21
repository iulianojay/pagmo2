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

#define BOOST_TEST_MODULE mpi_island_test
#include <boost/test/unit_test.hpp>

#include <mpi.h>

#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/de.hpp>
#include <pagmo/island.hpp>
#include <pagmo/islands/mpi_island.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/problems/schwefel.hpp>

using namespace pagmo;

// MPI initialization fixture
struct mpi_fixture {
    mpi_fixture()
    {
        int initialized;
        MPI_Initialized(&initialized);
        if (!initialized) {
            int argc = 0;
            char **argv = nullptr;
            MPI_Init(&argc, &argv);
        }
    }
    ~mpi_fixture()
    {
        int finalized;
        MPI_Finalized(&finalized);
        if (!finalized) {
            MPI_Finalize();
        }
    }
};

BOOST_GLOBAL_FIXTURE(mpi_fixture);

BOOST_AUTO_TEST_CASE(mpi_island_construction)
{
    // Default construction
    mpi_island udi;
    BOOST_CHECK_EQUAL(udi.get_name(), "MPI island");

    // Construction with custom chunk size
    mpi_island udi_custom(MPI_COMM_WORLD, 512 * 1024);
    BOOST_CHECK_EQUAL(udi_custom.get_chunk_size(), 512 * 1024);

    // Copy construction
    mpi_island udi_copy(udi_custom);
    BOOST_CHECK_EQUAL(udi_copy.get_chunk_size(), udi_custom.get_chunk_size());
    BOOST_CHECK_EQUAL(udi_copy.get_size(), udi_custom.get_size());

    // Move construction
    size_t chunk = udi_custom.get_chunk_size();
    mpi_island udi_move(std::move(udi_custom));
    BOOST_CHECK_EQUAL(udi_move.get_chunk_size(), chunk);
}

BOOST_AUTO_TEST_CASE(mpi_island_queries)
{
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    mpi_island udi;

    BOOST_CHECK_EQUAL(udi.get_rank(), rank);
    BOOST_CHECK_EQUAL(udi.get_size(), size);
    BOOST_CHECK_EQUAL(udi.is_master(), (rank == 0));
}

BOOST_AUTO_TEST_CASE(mpi_island_basic_evolution)
{
    problem prob{rosenbrock{10}};
    population pop{prob, 20};
    algorithm algo{de{50}};

    double initial_f = pop.get_f()[pop.best_idx()][0];

    island isl{mpi_island{}, algo, pop};
    isl.evolve();
    isl.wait();

    double final_f = isl.get_population().get_f()[isl.get_population().best_idx()][0];

    // Should improve
    BOOST_CHECK(final_f < initial_f);
}

BOOST_AUTO_TEST_CASE(mpi_island_evolution_schwefel)
{
    problem prob{schwefel{30}};
    population pop{prob, 40};
    algorithm algo{de{100}};

    double initial_f = pop.get_f()[pop.best_idx()][0];

    island isl{mpi_island{}, algo, pop};
    isl.evolve();
    isl.wait();

    double final_f = isl.get_population().get_f()[isl.get_population().best_idx()][0];

    // Should improve
    BOOST_CHECK(final_f < initial_f);
}

BOOST_AUTO_TEST_CASE(mpi_island_multiple_evolutions)
{
    problem prob{rosenbrock{5}};
    population pop{prob, 20};
    algorithm algo{de{20}};

    island isl{mpi_island{}, algo, pop};

    double prev_f = pop.get_f()[pop.best_idx()][0];

    for (int i = 0; i < 3; ++i) {
        isl.evolve();
        isl.wait();

        double curr_f = isl.get_population().get_f()[isl.get_population().best_idx()][0];

        // Should keep improving or stay same
        BOOST_CHECK(curr_f <= prev_f);
        prev_f = curr_f;
    }
}

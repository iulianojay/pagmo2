/* Example: Using MPI Island for Massively Parallel Optimization
 *
 * This example demonstrates how to use pagmo's mpi_island to distribute
 * optimization work across multiple MPI processes.
 *
 * Compile with:
 *   mpicxx -o mpi_island_example mpi_island_example.cpp -lpagmo -std=c++17
 *
 * Run with:
 *   mpirun -np 8 ./mpi_island_example
 *
 * This will use 8 MPI ranks: 1 master + 7 workers
 */

#include <iostream>
#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/de.hpp>
#include <pagmo/island.hpp>
#include <pagmo/islands/mpi_island.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/problems/rosenbrock.hpp>

#include <mpi.h>

int main(int argc, char *argv[])
{
    // Initialize MPI
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    try {
        // Only master prints header
        if (rank == 0) {
            std::cout << "======================================\n";
            std::cout << "  PaGMO MPI Island Example\n";
            std::cout << "======================================\n";
            std::cout << "Number of MPI ranks: " << size << "\n";
            std::cout << "Master rank: 0\n";
            std::cout << "Worker ranks: 1-" << (size - 1) << "\n\n";
        }

        // Create the optimization problem (30-dimensional Rosenbrock)
        pagmo::problem prob{pagmo::rosenbrock{30}};

        // Create an island with MPI backend
        pagmo::mpi_island udi;

        if (rank == 0) {
            std::cout << "Island info:\n" << udi.get_extra_info() << "\n\n";
        }

        // Create a population with 40 individuals
        pagmo::population pop{prob, 40};

        if (rank == 0) {
            std::cout << "Initial best fitness: " << pop.get_f()[pop.best_idx()][0] << "\n";
        }

        // Create an algorithm (Differential Evolution)
        pagmo::algorithm algo{pagmo::de{10000}}; // 100 generations

        // Create the island
        pagmo::island isl{udi, algo, pop};

        if (rank == 0) {
            std::cout << "Starting evolution across " << size << " MPI ranks...\n";
        }

        // Evolve - this will distribute work across all MPI ranks
        isl.evolve();
        isl.wait(); // Wait for evolution to complete

        // All ranks participate in the evolution
        // Only master prints results
        if (rank == 0) {
            std::cout << "Evolution complete!\n";
            std::cout << "Final best fitness: " << isl.get_population().get_f()[isl.get_population().best_idx()][0]
                      << "\n";
            std::cout << "Champion decision vector:\n";
            auto x = isl.get_population().get_x()[isl.get_population().best_idx()];
            for (size_t i = 0; i < x.size(); ++i) {
                std::cout << "  x[" << i << "] = " << x[i] << "\n";
            }
        }

    } catch (const std::exception &e) {
        std::cerr << "Error on rank " << rank << ": " << e.what() << "\n";
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Finalize MPI
    MPI_Finalize();

    return 0;
}

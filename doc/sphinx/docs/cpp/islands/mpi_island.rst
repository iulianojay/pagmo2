MPI island
==========

.. versionadded:: 2.20

*#include <pagmo/islands/mpi_island.hpp>*

.. note::

   The :cpp:class:`~pagmo::mpi_island` class is available only when pagmo has been compiled with MPI support
   (i.e., when the ``PAGMO_WITH_MPI`` preprocessor constant is defined). MPI support can be enabled by passing
   ``-DPAGMO_WITH_MPI=ON`` to CMake during configuration.

.. cpp:namespace-push:: pagmo

.. cpp:class:: mpi_island

   MPI island.

   This user-defined island (UDI) provides massively parallel optimization capabilities by distributing
   evolutionary algorithm computations across multiple processes using the Message Passing Interface (MPI).
   :cpp:class:`~pagmo::mpi_island` is designed to scale to hundreds or thousands of CPU cores on HPC systems.

   :cpp:class:`~pagmo::mpi_island` implements a master-worker architecture where rank 0 acts as the master,
   managing the island and population, while ranks 1-N act as workers that perform the actual evolution
   in parallel. The master distributes serialized algorithm and population data to all workers using
   chunked MPI communication, workers evolve their local copies independently, and the best result is
   returned to the master.

   Unlike :cpp:class:`~pagmo::thread_island` which uses shared memory parallelism, or
   :cpp:class:`~pagmo::fork_island` which creates separate processes on a single machine,
   :cpp:class:`~pagmo::mpi_island` enables distributed-memory parallelism across multiple compute nodes,
   making it suitable for:

   * Large-scale HPC cluster deployments (100+ cores)
   * Expensive fitness function evaluations
   * Problems requiring massive parallel search
   * Distributed optimization campaigns

   **Important**: All MPI ranks must participate in the evolution. When :cpp:func:`run_evolve()` is called
   on the master rank, all worker ranks must also be ready to participate in the collective MPI operations.
   User code should be structured so that all ranks create islands and call :cpp:func:`pagmo::island::evolve()`
   together.

   **Performance considerations**:

   * Works best with larger populations (50+ individuals) and expensive fitness functions
   * Communication overhead is minimal for typical optimization problems
   * Chunked communication (configurable) prevents MPI message size limitations
   * Scales efficiently to 1000+ cores on high-performance interconnects

   **MPI initialization**: User code must call ``MPI_Init()`` before creating :cpp:class:`~pagmo::mpi_island`
   instances and ``MPI_Finalize()`` at program termination.

   .. note::

      The algorithm and problem used with :cpp:class:`~pagmo::mpi_island` must be serializable using
      Boost.Serialization, as they need to be transmitted between MPI processes. Most standard pagmo
      algorithms and problems satisfy this requirement.

   .. cpp:function:: mpi_island()

      Default constructor.

      Constructs an MPI island using ``MPI_COMM_WORLD`` as the communicator and a default chunk size
      of 1 MB (1048576 bytes) for communication.

      :exception std\:\:runtime_error: if MPI has not been initialized via ``MPI_Init()``.

   .. cpp:function:: explicit mpi_island(MPI_Comm comm)

      Constructor with custom MPI communicator.

      This constructor allows the use of a custom MPI communicator, which is useful for creating
      sub-groups of processes or isolating island communications in an archipelago.

      :param comm: the MPI communicator to use for this island.

      :exception std\:\:runtime_error: if MPI has not been initialized via ``MPI_Init()``.

   .. cpp:function:: mpi_island(MPI_Comm comm, std::size_t chunk_size)

      Constructor with custom MPI communicator and chunk size.

      This constructor allows full control over the MPI communicator and the chunk size used for
      splitting large messages during serialization transfer.

      :param comm: the MPI communicator to use for this island.
      :param chunk_size: the size in bytes for each communication chunk (must be > 0).

      :exception std\:\:invalid_argument: if *chunk_size* is zero.
      :exception std\:\:runtime_error: if MPI has not been initialized via ``MPI_Init()``.

   .. cpp:function:: mpi_island(const mpi_island &other)
   .. cpp:function:: mpi_island(mpi_island &&other) noexcept

      :cpp:class:`~pagmo::mpi_island` is copy and move-constructible. Copy and move construction
      create a new island instance with the same configuration (chunk size, communicator) but in
      an inactive state.

   .. cpp:function:: void run_evolve(island &isl) const

      Run an evolution.

      This method implements the master-worker parallel evolution pattern:

      * **Master (rank 0)**:

        1. Serializes the algorithm and population from *isl*
        2. Broadcasts serialized data to all worker ranks in chunks
        3. Waits to receive the best evolved population from workers
        4. Updates *isl* with the best population and algorithm state

      * **Workers (rank 1-N)**:

        1. Receive serialized algorithm and population from master
        2. Deserialize and evolve the population locally
        3. Serialize the evolved population
        4. Send results back to master

      If any worker encounters an error during evolution, the error message is transmitted
      back to the master rank where a ``std::runtime_error`` will be raised.

      :param isl: the :cpp:class:`~pagmo::island` that will undergo evolution.

      :exception std\:\:runtime_error: if an MPI error occurs, or if any worker process
         encounters an error during evolution.
      :exception unspecified: any exception thrown by:

        * serialization/deserialization of the algorithm or population,
        * :cpp:func:`pagmo::algorithm::evolve()`,
        * :cpp:func:`pagmo::island::set_population()` or :cpp:func:`pagmo::island::set_algorithm()`.

   .. cpp:function:: std::string get_name() const

      Island's name.

      :return: the string ``"MPI island"``.

   .. cpp:function:: std::string get_extra_info() const

      Island's extra info.

      :return: a string containing information about the island's configuration, including
         the MPI rank, communicator size, chunk size, and whether this is the master rank.

   .. cpp:function:: int get_rank() const

      Get MPI rank.

      :return: the rank of this process in the MPI communicator.

   .. cpp:function:: int get_size() const

      Get MPI communicator size.

      :return: the total number of processes in the MPI communicator.

   .. cpp:function:: bool is_master() const

      Check if master rank.

      :return: ``true`` if this is rank 0 (the master), ``false`` otherwise.

   .. cpp:function:: bool is_active() const

      Check if evolution is active.

      :return: ``true`` if an evolution is currently in progress, ``false`` otherwise.

   .. cpp:function:: std::size_t get_chunk_size() const

      Get communication chunk size.

      :return: the current chunk size in bytes used for MPI communication.

   .. cpp:function:: void set_chunk_size(std::size_t size)

      Set communication chunk size.

      For very large populations, increasing the chunk size can improve communication efficiency.
      The chunk size determines how large messages are split during MPI send/receive operations.

      :param size: the new chunk size in bytes (must be > 0).

      :exception std\:\:invalid_argument: if *size* is zero.

Usage Example
-------------

Basic usage with default settings:

.. code-block:: c++

   #include <pagmo/algorithm.hpp>
   #include <pagmo/algorithms/de.hpp>
   #include <pagmo/island.hpp>
   #include <pagmo/islands/mpi_island.hpp>
   #include <pagmo/population.hpp>
   #include <pagmo/problem.hpp>
   #include <pagmo/problems/rosenbrock.hpp>
   #include <mpi.h>
   #include <iostream>

   int main(int argc, char *argv[])
   {
       // Initialize MPI
       MPI_Init(&argc, &argv);

       // Get rank information
       int rank;
       MPI_Comm_rank(MPI_COMM_WORLD, &rank);

       // Create problem and population
       pagmo::problem prob{pagmo::rosenbrock{30}};
       pagmo::population pop{prob, 40};

       // Create algorithm
       pagmo::algorithm algo{pagmo::de{100}};

       // Create island with MPI backend
       pagmo::island isl{pagmo::mpi_island{}, algo, pop};

       // Evolve (all ranks participate automatically)
       isl.evolve();
       isl.wait();

       // Master rank can access results
       if (rank == 0) {
           auto best_idx = isl.get_population().best_idx();
           auto best_f = isl.get_population().get_f()[best_idx][0];
           std::cout << "Best fitness: " << best_f << std::endl;
       }

       // Finalize MPI
       MPI_Finalize();
       return 0;
   }

To compile and run:

.. code-block:: bash

   # Compile with MPI compiler wrapper
   mpicxx -o my_optimizer my_optimizer.cpp -lpagmo -std=c++17

   # Run with 16 MPI processes (1 master + 15 workers)
   mpirun -np 16 ./my_optimizer

Building pagmo with MPI Support
--------------------------------

To build pagmo with MPI island support:

.. code-block:: bash

   mkdir build && cd build
   cmake .. -DPAGMO_WITH_MPI=ON
   make
   sudo make install

Or using the provided Makefile:

.. code-block:: bash

   make mpi

Prerequisites:

* An MPI implementation (OpenMPI, MPICH, Intel MPI, etc.)
* C++17 compatible compiler
* Boost libraries with serialization support
* Intel TBB

On Ubuntu/Debian:

.. code-block:: bash

   sudo apt-get install libopenmpi-dev openmpi-bin

Performance Guidelines
----------------------

For optimal performance with :cpp:class:`~pagmo::mpi_island`:

* **Population size**: Use larger populations (50-200 individuals). Very small populations (< 20) may not justify
  the communication overhead.

* **Fitness function cost**: Best suited for expensive fitness functions where evaluation time dominates
  communication time. For very cheap fitness functions, consider :cpp:class:`~pagmo::thread_island` instead.

* **Number of processes**: The master-worker pattern uses 1 master + N workers. Common configurations:

  - 8-16 processes: Development and small-scale testing
  - 32-64 processes: Departmental clusters
  - 128-512 processes: Medium HPC systems
  - 512+ processes: Large HPC systems with fast interconnects

* **Network**: High-performance interconnects (InfiniBand, Omni-Path) are recommended for deployments
  with 100+ processes. Standard Ethernet is adequate for smaller scales.

* **Chunk size**: The default 1 MB chunk size works well for most cases. Increase it for very large
  populations (500+ individuals) using :cpp:func:`set_chunk_size()`.

.. cpp:namespace-pop::

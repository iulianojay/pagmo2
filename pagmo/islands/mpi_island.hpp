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

#ifndef PAGMO_ISLANDS_MPI_ISLAND_HPP
#define PAGMO_ISLANDS_MPI_ISLAND_HPP

#include <pagmo/config.hpp>

#if defined(PAGMO_WITH_MPI)

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include <mpi.h>

#include <pagmo/detail/visibility.hpp>
#include <pagmo/island.hpp>
#include <pagmo/s11n.hpp>

namespace pagmo
{

// MPI island: offloads evolution to MPI worker processes for massive parallelism.
// This implementation is designed to scale to hundreds or thousands of cores.
//
// Design:
// - Rank 0 acts as the master, distributing work to worker ranks
// - Workers evolve populations and return results
// - Supports both synchronous and asynchronous evolution
// - Uses optimized serialization and chunked communication for large populations
// - Automatically handles load balancing across available workers
//
// Usage:
// - MPI must be initialized before using this island (MPI_Init/MPI_Init_thread)
// - All ranks must participate in the run_evolve() call
// - The island uses MPI_COMM_WORLD by default, but can use custom communicators
//
class PAGMO_DLL_PUBLIC mpi_island
{
public:
    // MPI tags for different message types
    enum class msg_tag : int {
        work_request = 1,    // Worker requests work
        work_data = 2,       // Master sends work data
        result_data = 3,     // Worker sends results
        terminate = 4,       // Master signals termination
        error = 5            // Worker reports error
    };

    // Default constructor - uses MPI_COMM_WORLD
    mpi_island();

    // Constructor with custom communicator
    // NOTE: The communicator is duplicated internally, so the caller retains ownership
    explicit mpi_island(MPI_Comm comm);

    // Constructor with communicator and chunk size
    // chunk_size controls how many bytes to send per MPI message (for large populations)
    mpi_island(MPI_Comm comm, size_t chunk_size);

    // Copy constructor - duplicates the communicator
    mpi_island(const mpi_island &);

    // Move constructor
    mpi_island(mpi_island &&) noexcept;

    // Copy assignment
    mpi_island &operator=(const mpi_island &);

    // Move assignment
    mpi_island &operator=(mpi_island &&) noexcept;

    // Destructor - frees the communicator
    ~mpi_island();

    // Run evolution using MPI
    void run_evolve(island &) const;

    // Island's name
    std::string get_name() const
    {
        return "MPI island";
    }

    // Extra info: reports MPI rank, size, and active status
    std::string get_extra_info() const;

    // Get the MPI rank within the island's communicator
    int get_rank() const;

    // Get the size of the MPI communicator
    int get_size() const;

    // Get whether this rank is the master (rank 0)
    bool is_master() const
    {
        return get_rank() == 0;
    }

    // Get whether evolution is currently active
    bool is_active() const
    {
        return m_active.load();
    }

    // Set the chunk size for communication (in bytes)
    void set_chunk_size(size_t chunk_size)
    {
        m_chunk_size = chunk_size;
    }

    // Get the current chunk size
    size_t get_chunk_size() const
    {
        return m_chunk_size;
    }

private:
    // Master process: distributes work and collects results
    void master_process(island &) const;

    // Worker process: receives work, evolves, and returns results
    void worker_process() const;

    // Send serialized data in chunks
    void send_chunked(const std::string &data, int dest, msg_tag tag) const;

    // Receive chunked data
    std::string recv_chunked(int source, msg_tag tag) const;

    // Object serialization
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &, unsigned);

    // MPI communicator (duplicated from the one passed in constructor)
    MPI_Comm m_comm;

    // Chunk size for large message transfers (default: 1 MB)
    size_t m_chunk_size;

    // Active flag (set during evolution)
    mutable std::atomic<bool> m_active;

    // Rank cache (to avoid repeated MPI_Comm_rank calls)
    mutable int m_rank_cache;

    // Size cache
    mutable int m_size_cache;

    // Flag indicating if caches are valid
    mutable bool m_cache_valid;
};

} // namespace pagmo

PAGMO_S11N_ISLAND_EXPORT_KEY(pagmo::mpi_island)

#else

#error The mpi_island.hpp header was included, but MPI support is not available. Build with -DPAGMO_WITH_MPI=ON

#endif

#endif

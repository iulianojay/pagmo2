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

#include <algorithm>
#include <cstring>
#include <exception>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <mpi.h>

#include <pagmo/algorithm.hpp>
#include <pagmo/exceptions.hpp>
#include <pagmo/island.hpp>
#include <pagmo/islands/mpi_island.hpp>
#include <pagmo/population.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/utils/multi_objective.hpp>

namespace pagmo
{

namespace detail
{

namespace
{

// Helper to check MPI initialization
void check_mpi_init()
{
    int initialized = 0;
    MPI_Initialized(&initialized);
    if (!initialized) {
        pagmo_throw(std::runtime_error,
                    "MPI has not been initialized. Please call MPI_Init() or MPI_Init_thread() "
                    "before using mpi_island");
    }
}

// Helper to get MPI error string
std::string get_mpi_error_string(int error_code)
{
    char error_string[MPI_MAX_ERROR_STRING];
    int length;
    MPI_Error_string(error_code, error_string, &length);
    return std::string(error_string, static_cast<size_t>(length));
}

// RAII wrapper for MPI error handling
struct mpi_error_checker {
    explicit mpi_error_checker(int error_code, const std::string &operation) : m_error_code(error_code), m_op(operation)
    {
        if (m_error_code != MPI_SUCCESS) {
            pagmo_throw(std::runtime_error, "MPI error in " + m_op + ": " + get_mpi_error_string(m_error_code));
        }
    }
    int m_error_code;
    std::string m_op;
};

// Serialize an object to a string using Boost serialization
template <typename T>
std::string serialize_to_string(const T &obj)
{
    std::stringstream ss;
    {
        boost::archive::binary_oarchive oarchive(ss);
        oarchive << obj;
    }
    return ss.str();
}

// Deserialize an object from a string
template <typename T>
T deserialize_from_string(const std::string &data)
{
    std::stringstream ss(data);
    boost::archive::binary_iarchive iarchive(ss);
    T obj;
    iarchive >> obj;
    return obj;
}

} // namespace

} // namespace detail

// Default constructor
mpi_island::mpi_island() : mpi_island(MPI_COMM_WORLD) {}

// Constructor with custom communicator
mpi_island::mpi_island(MPI_Comm comm) : mpi_island(comm, 1024 * 1024) {} // 1 MB default chunk size

// Constructor with communicator and chunk size
mpi_island::mpi_island(MPI_Comm comm, size_t chunk_size)
    : m_comm(MPI_COMM_NULL), m_chunk_size(chunk_size), m_active(false), m_rank_cache(-1), m_size_cache(-1),
      m_cache_valid(false)
{
    detail::check_mpi_init();

    // Duplicate the communicator so we have our own copy
    int error = MPI_Comm_dup(comm, &m_comm);
    detail::mpi_error_checker(error, "MPI_Comm_dup in mpi_island constructor");

    // Cache rank and size
    MPI_Comm_rank(m_comm, &m_rank_cache);
    MPI_Comm_size(m_comm, &m_size_cache);
    m_cache_valid = true;
}

// Copy constructor
mpi_island::mpi_island(const mpi_island &other)
    : m_comm(MPI_COMM_NULL), m_chunk_size(other.m_chunk_size), m_active(false), m_rank_cache(-1), m_size_cache(-1),
      m_cache_valid(false)
{
    if (other.m_comm != MPI_COMM_NULL) {
        int error = MPI_Comm_dup(other.m_comm, &m_comm);
        detail::mpi_error_checker(error, "MPI_Comm_dup in mpi_island copy constructor");

        MPI_Comm_rank(m_comm, &m_rank_cache);
        MPI_Comm_size(m_comm, &m_size_cache);
        m_cache_valid = true;
    }
}

// Move constructor
mpi_island::mpi_island(mpi_island &&other) noexcept
    : m_comm(other.m_comm), m_chunk_size(other.m_chunk_size), m_active(other.m_active.load()),
      m_rank_cache(other.m_rank_cache), m_size_cache(other.m_size_cache), m_cache_valid(other.m_cache_valid)
{
    other.m_comm = MPI_COMM_NULL;
    other.m_cache_valid = false;
}

// Copy assignment
mpi_island &mpi_island::operator=(const mpi_island &other)
{
    if (this != &other) {
        // Free existing communicator
        if (m_comm != MPI_COMM_NULL) {
            MPI_Comm_free(&m_comm);
        }

        // Duplicate the other's communicator
        m_comm = MPI_COMM_NULL;
        m_chunk_size = other.m_chunk_size;
        m_active.store(false);
        m_cache_valid = false;

        if (other.m_comm != MPI_COMM_NULL) {
            int error = MPI_Comm_dup(other.m_comm, &m_comm);
            detail::mpi_error_checker(error, "MPI_Comm_dup in mpi_island copy assignment");

            MPI_Comm_rank(m_comm, &m_rank_cache);
            MPI_Comm_size(m_comm, &m_size_cache);
            m_cache_valid = true;
        }
    }
    return *this;
}

// Move assignment
mpi_island &mpi_island::operator=(mpi_island &&other) noexcept
{
    if (this != &other) {
        // Free existing communicator
        if (m_comm != MPI_COMM_NULL) {
            MPI_Comm_free(&m_comm);
        }

        // Take ownership of other's resources
        m_comm = other.m_comm;
        m_chunk_size = other.m_chunk_size;
        m_active.store(other.m_active.load());
        m_rank_cache = other.m_rank_cache;
        m_size_cache = other.m_size_cache;
        m_cache_valid = other.m_cache_valid;

        other.m_comm = MPI_COMM_NULL;
        other.m_cache_valid = false;
    }
    return *this;
}

// Destructor
mpi_island::~mpi_island()
{
    if (m_comm != MPI_COMM_NULL) {
        MPI_Comm_free(&m_comm);
    }
}

// Get rank
int mpi_island::get_rank() const
{
    if (!m_cache_valid) {
        MPI_Comm_rank(m_comm, &m_rank_cache);
        m_cache_valid = true;
    }
    return m_rank_cache;
}

// Get size
int mpi_island::get_size() const
{
    if (!m_cache_valid) {
        MPI_Comm_size(m_comm, &m_size_cache);
        m_cache_valid = true;
    }
    return m_size_cache;
}

// Extra info
std::string mpi_island::get_extra_info() const
{
    std::string info = "\tMPI rank: " + std::to_string(get_rank()) + "/" + std::to_string(get_size());
    if (is_active()) {
        info += "\n\tStatus: evolving";
    } else {
        info += "\n\tStatus: idle";
    }
    info += "\n\tChunk size: " + std::to_string(m_chunk_size) + " bytes";
    return info;
}

// Send data in chunks
void mpi_island::send_chunked(const std::string &data, int dest, msg_tag tag) const
{
    // First, send the total size
    size_t total_size = data.size();
    int error = MPI_Send(&total_size, 1, MPI_UNSIGNED_LONG, dest, static_cast<int>(tag), m_comm);
    detail::mpi_error_checker(error, "MPI_Send (size) in send_chunked");

    // Send data in chunks
    size_t offset = 0;
    while (offset < total_size) {
        size_t chunk = std::min(m_chunk_size, total_size - offset);
        error = MPI_Send(data.data() + offset, static_cast<int>(chunk), MPI_BYTE, dest, static_cast<int>(tag), m_comm);
        detail::mpi_error_checker(error, "MPI_Send (data) in send_chunked");
        offset += chunk;
    }
}

// Receive data in chunks
std::string mpi_island::recv_chunked(int source, msg_tag tag) const
{
    // First, receive the total size
    size_t total_size = 0;
    MPI_Status status;
    int error = MPI_Recv(&total_size, 1, MPI_UNSIGNED_LONG, source, static_cast<int>(tag), m_comm, &status);
    detail::mpi_error_checker(error, "MPI_Recv (size) in recv_chunked");

    // Allocate buffer for the full message
    std::string data(total_size, '\0');

    // Receive data in chunks
    size_t offset = 0;
    while (offset < total_size) {
        size_t chunk = std::min(m_chunk_size, total_size - offset);
        error = MPI_Recv(&data[offset], static_cast<int>(chunk), MPI_BYTE, source, static_cast<int>(tag), m_comm,
                         &status);
        detail::mpi_error_checker(error, "MPI_Recv (data) in recv_chunked");
        offset += chunk;
    }

    return data;
}

// Master process implementation
void mpi_island::master_process(island &isl) const
{
    const int size = get_size();

    // If we only have one rank, just evolve locally
    if (size == 1) {
        auto algo = isl.get_algorithm();
        auto pop = isl.get_population();
        isl.set_population(algo.evolve(pop));
        isl.set_algorithm(algo);
        return;
    }

    // Serialize the algorithm and population
    std::string algo_data, pop_data;
    try {
        algo_data = detail::serialize_to_string(isl.get_algorithm());
        pop_data = detail::serialize_to_string(isl.get_population());
    } catch (const std::exception &e) {
        pagmo_throw(std::runtime_error,
                    "Failed to serialize algorithm or population in mpi_island master: " + std::string(e.what()));
    }

    // Send work to all workers (ranks 1 to size-1)
    for (int worker = 1; worker < size; ++worker) {
        try {
            send_chunked(algo_data, worker, msg_tag::work_data);
            send_chunked(pop_data, worker, msg_tag::work_data);
        } catch (const std::exception &e) {
            // If sending fails, send terminate signal to all workers and abort
            for (int w = 1; w < size; ++w) {
                int dummy = 0;
                MPI_Send(&dummy, 1, MPI_INT, w, static_cast<int>(msg_tag::terminate), m_comm);
            }
            throw;
        }
    }

    // For now, we collect results from all workers and use the best one
    // In a more sophisticated implementation, you could:
    // - Use only the first result and cancel others
    // - Aggregate results from multiple workers
    // - Implement a voting/consensus mechanism
    std::vector<std::tuple<algorithm, population>> results;
    results.reserve(size - 1);

    for (int worker = 1; worker < size; ++worker) {
        try {
            // Check if worker is sending an error
            MPI_Status status;
            MPI_Probe(worker, MPI_ANY_TAG, m_comm, &status);

            if (status.MPI_TAG == static_cast<int>(msg_tag::error)) {
                std::string error_msg = recv_chunked(worker, msg_tag::error);
                pagmo_throw(std::runtime_error,
                            "Worker " + std::to_string(worker) + " reported error: " + error_msg);
            }

            // Receive result
            std::string result_algo_data = recv_chunked(worker, msg_tag::result_data);
            std::string result_pop_data = recv_chunked(worker, msg_tag::result_data);

            algorithm result_algo = detail::deserialize_from_string<algorithm>(result_algo_data);
            population result_pop = detail::deserialize_from_string<population>(result_pop_data);

            results.emplace_back(std::move(result_algo), std::move(result_pop));
        } catch (const std::exception &e) {
            // Log error but continue collecting from other workers
            // In production, you might want to handle this more gracefully
            throw;
        }
    }

    // Select the best set of solutions maintaining the original population size
    if (!results.empty()) {
        // Get the original population size and problem
        const auto original_pop = isl.get_population();
        const auto original_size = original_pop.size();
        const auto &prob = original_pop.get_problem();
        const auto nobj = prob.get_nobj();

        // Combine all fitness vectors and decision vectors from all worker results
        std::vector<vector_double> combined_f;
        std::vector<vector_double> combined_x;
        std::vector<size_t> result_indices; // Track which result each individual came from

        combined_f.reserve(original_size * results.size());
        combined_x.reserve(original_size * results.size());
        result_indices.reserve(original_size * results.size());

        for (size_t i = 0; i < results.size(); ++i) {
            const auto &pop = std::get<1>(results[i]);
            const auto &f_vecs = pop.get_f();
            const auto &x_vecs = pop.get_x();

            for (size_t j = 0; j < f_vecs.size(); ++j) {
                combined_f.push_back(f_vecs[j]);
                combined_x.push_back(x_vecs[j]);
                result_indices.push_back(i);
            }
        }

        // Select the best N individuals based on whether problem is multi-objective
        std::vector<pop_size_t> best_indices;

        if (nobj > 1) {
            // Multi-objective: use select_best_N_mo which handles Pareto dominance
            best_indices = select_best_N_mo(combined_f, original_size);
        } else {
            // Single-objective: sort by fitness and select the best N
            std::vector<size_t> sorted_indices(combined_f.size());
            std::iota(sorted_indices.begin(), sorted_indices.end(), 0);

            std::sort(sorted_indices.begin(), sorted_indices.end(),
                      [&combined_f](size_t a, size_t b) {
                          return combined_f[a][0] < combined_f[b][0];
                      });

            // Take the first original_size individuals
            best_indices.reserve(original_size);
            for (size_t i = 0; i < original_size && i < sorted_indices.size(); ++i) {
                best_indices.push_back(sorted_indices[i]);
            }
        }

        // Build a new population with the selected individuals
        population new_pop(prob);
        for (auto idx : best_indices) {
            new_pop.push_back(combined_x[idx], combined_f[idx]);
        }

        // Select the algorithm from the result that contributed the most individuals
        std::vector<size_t> contribution_counts(results.size(), 0);
        for (auto idx : best_indices) {
            contribution_counts[result_indices[idx]]++;
        }

        auto best_contrib_it = std::max_element(contribution_counts.begin(), contribution_counts.end());
        size_t best_result_idx = std::distance(contribution_counts.begin(), best_contrib_it);

        // Update the island with the new population and selected algorithm
        isl.set_algorithm(std::get<0>(results[best_result_idx]));
        isl.set_population(new_pop);
    }
}

// Worker process implementation
void mpi_island::worker_process() const
{
    try {
        // Receive algorithm and population from master
        std::string algo_data = recv_chunked(0, msg_tag::work_data);
        std::string pop_data = recv_chunked(0, msg_tag::work_data);

        // Deserialize
        algorithm algo = detail::deserialize_from_string<algorithm>(algo_data);
        population pop = detail::deserialize_from_string<population>(pop_data);

        // Evolve
        population evolved_pop = algo.evolve(pop);

        // Serialize results
        std::string result_algo_data = detail::serialize_to_string(algo);
        std::string result_pop_data = detail::serialize_to_string(evolved_pop);

        // Send results back to master
        send_chunked(result_algo_data, 0, msg_tag::result_data);
        send_chunked(result_pop_data, 0, msg_tag::result_data);

    } catch (const std::exception &e) {
        // Send error message to master
        try {
            std::string error_msg = e.what();
            send_chunked(error_msg, 0, msg_tag::error);
        } catch (...) {
            // If we can't even send the error message, there's nothing we can do
            // The master will timeout or detect the problem
        }
        throw;
    }
}

// Run evolution
void mpi_island::run_evolve(island &isl) const
{
    // Set active flag
    m_active.store(true);

    // Ensure we clear the active flag on exit
    struct active_guard {
        explicit active_guard(std::atomic<bool> &flag) : m_flag(flag) {}
        ~active_guard()
        {
            m_flag.store(false);
        }
        std::atomic<bool> &m_flag;
    };
    active_guard guard(m_active);

    // Barrier to ensure all ranks are ready
    int error = MPI_Barrier(m_comm);
    detail::mpi_error_checker(error, "MPI_Barrier in run_evolve");

    const int rank = get_rank();

    if (rank == 0) {
        // Master process
        master_process(isl);
    } else {
        // Worker process
        worker_process();
    }

    // Final barrier to ensure all ranks complete
    error = MPI_Barrier(m_comm);
    detail::mpi_error_checker(error, "MPI_Barrier in run_evolve (final)");
}

// Serialization
template <typename Archive>
void mpi_island::serialize(Archive &ar, unsigned)
{
    // Note: We don't serialize the MPI communicator itself, as it's not portable
    // across processes. When deserializing, we'll create a new communicator.
    // We only serialize the chunk size configuration.
    ar &m_chunk_size;

    // On load, we need to reinitialize with MPI_COMM_WORLD
    // This is a simplification - in a more sophisticated implementation,
    // you might want to store communicator information differently
}

} // namespace pagmo

PAGMO_S11N_ISLAND_IMPLEMENT(pagmo::mpi_island)

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
#include <cassert>
#include <cstddef>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <pagmo/exceptions.hpp>
#include <pagmo/io.hpp>
#include <pagmo/topologies/base_bgl_topology.hpp>
#include <pagmo/topology.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/cast.hpp>
namespace pagmo
{

namespace detail
{

namespace
{

// Small helpers to reduce typing when converting to/from std::size_t.
template <typename I>
std::size_t scast(I n)
{
    return numeric_cast<std::size_t>(n);
}

} // namespace

} // namespace detail

// Small helper function that checks that the input vertices are in the graph.
// It will throw otherwise.
void base_bgl_topology::unsafe_check_vertex_indices() const {}

template <typename... Args>
void base_bgl_topology::unsafe_check_vertex_indices(std::size_t idx, Args... others) const
{
    if (!m_graph.has_vertex(idx)) {
        pagmo_throw(index_error, "invalid vertex index in a graph topology: the index is " + std::to_string(idx)
                                     + ", but the number of vertices is only "
                                     + std::to_string(m_graph.vertex_count()));
    }
    unsafe_check_vertex_indices(others...);
}

graph_t base_bgl_topology::get_graph() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_graph;
}

graph_t base_bgl_topology::move_graph()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return std::move(m_graph);
}

void base_bgl_topology::set_graph(graph_t &&g)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_graph = std::move(g);
}

base_bgl_topology::base_bgl_topology(const base_bgl_topology &other) : m_graph(other.get_graph()) {}

base_bgl_topology::base_bgl_topology(base_bgl_topology &&other) noexcept : m_graph(other.move_graph()) {}

base_bgl_topology &base_bgl_topology::operator=(const base_bgl_topology &other)
{
    if (this != &other) {
        set_graph(other.get_graph());
    }
    return *this;
}

base_bgl_topology &base_bgl_topology::operator=(base_bgl_topology &&other) noexcept
{
    if (this != &other) {
        set_graph(other.move_graph());
    }
    return *this;
}

void base_bgl_topology::add_vertex()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_graph.add_vertex(0);
}

std::size_t base_bgl_topology::num_vertices() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_graph.vertex_count();
}

bool base_bgl_topology::unsafe_are_adjacent(std::size_t i, std::size_t j) const
{
    unsafe_check_vertex_indices(i, j);
    return m_graph.has_edge(i, j);
}

bool base_bgl_topology::are_adjacent(std::size_t i, std::size_t j) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return unsafe_are_adjacent(i, j);
}

void base_bgl_topology::add_edge(std::size_t i, std::size_t j, double w)
{
    detail::topology_check_edge_weight(w);

    std::lock_guard<std::mutex> lock(m_mutex);

    if (unsafe_are_adjacent(i, j)) {
        pagmo_throw(index_error, "cannot add an edge in a graph topology: there is already an edge connecting "
                                     + std::to_string(i) + " to " + std::to_string(j));
    }

    m_graph.add_edge(i, j, w);
}

void base_bgl_topology::remove_edge(std::size_t i, std::size_t j)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!unsafe_are_adjacent(i, j)) {
        pagmo_throw(index_error, "cannot remove an edge in a graph topology: there is no edge connecting "
                                     + std::to_string(i) + " to " + std::to_string(j));
    }
    m_graph.remove_edge(i, j);
}

void base_bgl_topology::set_all_weights(double w)
{
    detail::topology_check_edge_weight(w);

    std::lock_guard<std::mutex> lock(m_mutex);

    for (const auto &[edge_id, _] : m_graph.get_edges()) {
        m_graph.get_edge(edge_id.first, edge_id.second) = w;
    }
}

void base_bgl_topology::set_weight(std::size_t i, std::size_t j, double w)
{
    detail::topology_check_edge_weight(w);

    std::lock_guard<std::mutex> lock(m_mutex);

    unsafe_check_vertex_indices(i, j);

    if (!m_graph.has_edge(i, j)) {
        pagmo_throw(index_error, "cannot set the weight of an edge in a graph topology: the vertex " + std::to_string(i)
                                     + " is not connected to vertex " + std::to_string(j));
    }
    m_graph.get_edge(i, j) = w;
}

std::pair<std::vector<std::size_t>, vector_double> base_bgl_topology::get_connections(std::size_t i) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    unsafe_check_vertex_indices(i);

    std::pair<std::vector<std::size_t>, vector_double> retval;

    // Find all edges whose target is vertex i (incoming edges).
    for (const auto &[edge_id, weight] : m_graph.get_edges()) {
        if (edge_id.second == i) {
            retval.first.emplace_back(edge_id.first);
            retval.second.emplace_back(weight);
        }
    }
    return retval;
}

double base_bgl_topology::get_edge_weight(std::size_t i, std::size_t j) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    unsafe_check_vertex_indices(i, j);

    if (!m_graph.has_edge(i, j)) {
        pagmo_throw(index_error, "cannot get the weight of an edge in a graph topology: the vertex " + std::to_string(i)
                                     + " is not connected to vertex " + std::to_string(j));
    }
    return m_graph.get_edge(i, j);
}

std::string base_bgl_topology::get_extra_info() const
{
    std::ostringstream oss;

    {
        std::lock_guard<std::mutex> lock(m_mutex);

        oss << "\tNumber of vertices: " << m_graph.vertex_count() << '\n';
        oss << "\tNumber of edges: " << m_graph.edge_count() << '\n';
        oss << "\tAdjacency list:\n\n";

        // Collect vertex IDs and sort for consistent output.
        std::vector<graaf::vertex_id_t> vids;
        vids.reserve(m_graph.vertex_count());
        for (const auto &[vid, _] : m_graph.get_vertices()) {
            vids.push_back(vid);
        }
        std::sort(vids.begin(), vids.end());

        for (const auto vid : vids) {
            // Build sorted (target, weight) pairs for the outgoing edges.
            std::vector<std::pair<graaf::vertex_id_t, double>> adj;
            for (const auto nid : m_graph.get_neighbors(vid)) {
                adj.emplace_back(nid, m_graph.get_edge(vid, nid));
            }
            std::sort(adj.begin(), adj.end());

            oss << "\t\t" << vid << ": ";
            detail::stream_range(oss, adj.begin(), adj.end());
            oss << '\n';
        }
    }

    return oss.str();
}

graph_t base_bgl_topology::to_graph() const
{
    return get_graph();
}

} // namespace pagmo

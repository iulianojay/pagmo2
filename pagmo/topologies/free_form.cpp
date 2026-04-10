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

#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>

#include <pagmo/exceptions.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/topologies/base_bgl_topology.hpp>
#include <pagmo/topologies/free_form.hpp>
#include <pagmo/topology.hpp>

namespace pagmo
{

free_form::free_form() = default;
free_form::free_form(const free_form &) = default;
free_form::free_form(free_form &&) noexcept = default;

free_form::free_form(graph_t g)
{
    // NOTE: verify the values of the weights,
    // as the base graph topology maintains
    // correct weights as a class invariant.
    for (const auto &[edge_id, w] : g.get_edges()) {
        if (!std::isfinite(w) || w < 0. || w > 1.) {
            pagmo_throw(invalid_value_error,
                        "In the constructor of a free_form topology from a graph object, an invalid edge weight of "
                            + std::to_string(w) + " was detected (the weight must be in the [0., 1.] range)");
        }
    }

    set_graph(std::move(g));
}

free_form::free_form(const topology &t) : free_form(t.to_graph()) {}

// Add vertex.
void free_form::push_back()
{
    add_vertex();
}

// Topology name.
std::string free_form::get_name() const
{
    return "Free form";
}

} // namespace pagmo

PAGMO_S11N_TOPOLOGY_IMPLEMENT(pagmo::free_form)

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

#ifndef PAGMO_TOPOLOGIES_FREE_FORM_HPP
#define PAGMO_TOPOLOGIES_FREE_FORM_HPP

#include <concepts>
#include <string>
#include <type_traits>

#include <pagmo/concepts.hpp>
#include <pagmo/detail/pagmo.fwd.hpp>
#include <pagmo/detail/visibility.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/topologies/base_bgl_topology.hpp>
#include <pagmo/topology.hpp>
#include <pagmo/type_traits.hpp>

namespace pagmo
{

// Free-form topology.
class PAGMO_DLL_PUBLIC free_form : public base_bgl_topology
{
public:
    free_form();
    free_form(const free_form &);
    free_form(free_form &&) noexcept;

    explicit free_form(graph_t);
    explicit free_form(const topology &);
    template <typename T>
        requires(!std::same_as<T, free_form> && IsUdTopology<T>)
    explicit free_form(const T &t) : free_form(topology(t))
    {
    }

    void push_back();

    std::string get_name() const;

private:
    // Object serialization
    friend class cereal::access;
    template <typename Archive>
    void serialize(Archive &ar)
    {

        detail::archive(ar, cereal::base_class<base_bgl_topology>(this));
    }
};

} // namespace pagmo

PAGMO_S11N_TOPOLOGY_EXPORT_KEY(pagmo::free_form)

#endif

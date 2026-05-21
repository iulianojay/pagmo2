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

#ifndef PAGMO_S11N_HPP
#define PAGMO_S11N_HPP

#include <cstddef>
#include <locale>
#include <random>
#include <sstream>
#include <string>
#include <tuple>

// Ensure cereal's static registry objects (StaticObject<PolymorphicCasters>)
// have default visibility so they are shared between the shared library and
// any executable that links against it. Without this, -fvisibility-inlines-hidden
// would give each DSO its own copy of the registry, breaking polymorphic serialization.
#pragma GCC visibility push(default)
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/vector.hpp>
#pragma GCC visibility pop

#include <graaflib/graph.h>

#include <pagmo/detail/s11n_wrappers.hpp>

namespace pagmo
{

namespace detail
{

// Cereal handles tuple serialization automatically, so we don't need custom implementation

} // namespace detail

} // namespace pagmo

// Cereal serialization support for Mersenne twister engine
namespace cereal
{
template <class Archive, class UIntType, std::size_t w, std::size_t n, std::size_t m, std::size_t r, UIntType a,
          std::size_t u, UIntType d, std::size_t s, UIntType b, std::size_t t, UIntType c, std::size_t l, UIntType f>
void save(Archive &ar, std::mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f> const &e)
{
    std::ostringstream oss;
    // Use the "C" locale.
    oss.imbue(std::locale::classic());
    oss << e;
    ar(oss.str());
}

template <class Archive, class UIntType, std::size_t w, std::size_t n, std::size_t m, std::size_t r, UIntType a,
          std::size_t u, UIntType d, std::size_t s, UIntType b, std::size_t t, UIntType c, std::size_t l, UIntType f>
void load(Archive &ar, std::mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f> &e)
{
    std::istringstream iss;
    // Use the "C" locale.
    iss.imbue(std::locale::classic());
    std::string tmp;
    ar(tmp);
    iss.str(tmp);
    iss >> e;
}
} // namespace cereal

// Cereal serialization support for graaf directed_graph.
// Uses public API (get_vertices/get_edges/add_vertex/add_edge) since
// graaf's internals are private.
namespace cereal
{

template <typename Archive, typename VERTEX_T, typename EDGE_T>
void save(Archive &ar, const graaf::directed_graph<VERTEX_T, EDGE_T> &g)
{
    const auto &vertices = g.get_vertices();
    const auto nv = static_cast<std::size_t>(vertices.size());
    ar(nv);
    for (const auto &[id, v] : vertices) {
        ar(id, v);
    }
    const auto &edges = g.get_edges();
    const auto ne = static_cast<std::size_t>(edges.size());
    ar(ne);
    for (const auto &[edge_id, e] : edges) {
        ar(edge_id.first, edge_id.second, e);
    }
}

template <typename Archive, typename VERTEX_T, typename EDGE_T>
void load(Archive &ar, graaf::directed_graph<VERTEX_T, EDGE_T> &g)
{
    std::size_t nv{};
    ar(nv);
    for (std::size_t i = 0; i < nv; ++i) {
        graaf::vertex_id_t id{};
        VERTEX_T v{};
        ar(id, v);
        g.add_vertex(std::move(v), id);
    }
    std::size_t ne{};
    ar(ne);
    for (std::size_t i = 0; i < ne; ++i) {
        graaf::vertex_id_t src{}, dst{};
        EDGE_T e{};
        ar(src, dst, e);
        g.add_edge(src, dst, std::move(e));
    }
}

} // namespace cereal

#endif

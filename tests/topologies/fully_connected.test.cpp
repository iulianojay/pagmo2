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

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

#include <pagmo/exceptions.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/topologies/fully_connected.hpp>
#include <pagmo/topology.hpp>

using namespace pagmo;

void verify_fully_connected_topology(const fully_connected &f)
{
    const auto s = f.num_vertices();

    if (!s) {
        EXPECT_THROW(f.get_connections(0), index_error);

        return;
    }

    if (s == 1u) {
        EXPECT_TRUE(f.get_connections(0).first.empty());
        EXPECT_TRUE(f.get_connections(0).second.empty());

        EXPECT_THROW(f.get_connections(1), index_error);

        return;
    }

    const auto w = f.get_weight();

    for (std::size_t i = 0; i < s; ++i) {
        const auto conns = f.get_connections(i);

        EXPECT_TRUE(conns.first.size() == s - 1u);
        EXPECT_TRUE(conns.second.size() == s - 1u);

        EXPECT_TRUE(std::all_of(conns.second.begin(), conns.second.end(), [w](double x) { return x == w; }));

        for (std::size_t j = 0; j < i; ++j) {
            EXPECT_TRUE(std::find(conns.first.begin(), conns.first.end(), j) != conns.first.end());
        }
        for (std::size_t j = i + 1u; j < s; ++j) {
            EXPECT_TRUE(std::find(conns.first.begin(), conns.first.end(), j) != conns.first.end());
        }
    }
}

TEST(fully_connected, basic_test)
{
    {
        // Default construct, push back a few times.
        fully_connected r0;
        EXPECT_TRUE(r0.get_weight() == 1);
        EXPECT_TRUE(r0.num_vertices() == 0u);
        verify_fully_connected_topology(r0);

        r0.push_back();
        EXPECT_TRUE(r0.num_vertices() == 1u);
        verify_fully_connected_topology(r0);

        r0.push_back();
        EXPECT_TRUE(r0.num_vertices() == 2u);
        verify_fully_connected_topology(r0);

        r0.push_back();
        EXPECT_TRUE(r0.num_vertices() == 3u);
        verify_fully_connected_topology(r0);

        r0.push_back();
        r0.push_back();
        r0.push_back();
        r0.push_back();
        EXPECT_TRUE(r0.num_vertices() == 7u);
        verify_fully_connected_topology(r0);
    }

    {
        // Ctor from weight.
        fully_connected r0(.2);
        EXPECT_TRUE(r0.get_weight() == .2);
        EXPECT_TRUE(r0.num_vertices() == 0u);
        verify_fully_connected_topology(r0);

        r0.push_back();
        EXPECT_TRUE(r0.num_vertices() == 1u);
        verify_fully_connected_topology(r0);

        r0.push_back();
        EXPECT_TRUE(r0.num_vertices() == 2u);
        verify_fully_connected_topology(r0);

        r0.push_back();
        EXPECT_TRUE(r0.num_vertices() == 3u);
        verify_fully_connected_topology(r0);

        r0.push_back();
        r0.push_back();
        r0.push_back();
        r0.push_back();
        EXPECT_TRUE(r0.num_vertices() == 7u);
        verify_fully_connected_topology(r0);
    }

    {
        // Ctor from nedges and weight.
        fully_connected r0(0, .2);
        EXPECT_TRUE(r0.get_weight() == .2);
        EXPECT_TRUE(r0.num_vertices() == 0u);
        verify_fully_connected_topology(r0);
    }

    {
        // Ctor from nedges and weight.
        fully_connected r0(7, .2);
        EXPECT_TRUE(r0.get_weight() == .2);
        EXPECT_TRUE(r0.num_vertices() == 7u);
        verify_fully_connected_topology(r0);
    }

    {
        // Copy/move ctors.
        fully_connected r0(7, .2), r1(r0), r2(std::move(r0));

        EXPECT_TRUE(r1.get_weight() == .2);
        EXPECT_TRUE(r1.num_vertices() == 7u);

        EXPECT_TRUE(r1.get_weight() == .2);
        EXPECT_TRUE(r2.num_vertices() == 7u);

        verify_fully_connected_topology(r1);
        verify_fully_connected_topology(r2);
    }

    {
        // Name/extra info.
        fully_connected r0(7, .2);

        EXPECT_TRUE(r0.get_name() == "Fully connected");
        EXPECT_TRUE(r0.get_extra_info().contains("Edges' weight:"));

        std::cout << r0.get_extra_info() << '\n';
    }

    // Minimal serialization test.
    fully_connected r0(7, .2);
    {
        topology t0(r0);
        std::stringstream ss;
        {
            cereal::BinaryOutputArchive oarchive(ss);
            oarchive(t0);
        }
        topology t1;
        EXPECT_TRUE(!t1.is<fully_connected>());
        {
            cereal::BinaryInputArchive iarchive(ss);
            iarchive(t1);
        }
        EXPECT_TRUE(t1.is<fully_connected>());
        EXPECT_TRUE(t1.extract<fully_connected>()->num_vertices() == 7u);
        EXPECT_TRUE(t1.extract<fully_connected>()->get_weight() == .2);
        verify_fully_connected_topology(*t1.extract<fully_connected>());
    }
}

TEST(fully_connected, to_graph_test)
{
    EXPECT_TRUE(HasToGraph<fully_connected>);

    auto g0 = fully_connected{}.to_graph();
    EXPECT_TRUE(g0.vertex_count() == 0u);

    g0 = fully_connected{1, .5}.to_graph();
    EXPECT_TRUE(g0.vertex_count() == 1u);
    EXPECT_TRUE(g0.get_neighbors(0).empty());

    g0 = fully_connected{2, .5}.to_graph();
    EXPECT_TRUE(g0.vertex_count() == 2u);
    EXPECT_TRUE(g0.has_edge(0, 1));
    EXPECT_TRUE(g0.get_edge(0, 1) == .5);
    EXPECT_TRUE(g0.has_edge(1, 0));
    EXPECT_TRUE(g0.get_edge(1, 0) == .5);
    EXPECT_TRUE(g0.get_neighbors(0).size() == 1u);
    EXPECT_TRUE(g0.get_neighbors(1).size() == 1u);

    g0 = fully_connected{3, .5}.to_graph();
    EXPECT_TRUE(g0.vertex_count() == 3u);
    // Every vertex should have 2 outgoing edges to all others.
    EXPECT_TRUE(g0.get_neighbors(0).size() == 2u);
    EXPECT_TRUE(g0.get_neighbors(1).size() == 2u);
    EXPECT_TRUE(g0.get_neighbors(2).size() == 2u);
    // Check all cross-edges exist and have correct weight.
    for (std::size_t i = 0; i < 3u; ++i) {
        for (std::size_t j = 0; j < 3u; ++j) {
            if (i != j) {
                EXPECT_TRUE(g0.has_edge(i, j));
                EXPECT_TRUE(g0.get_edge(i, j) == .5);
            }
        }
    }
}

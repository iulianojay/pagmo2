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

#include <atomic>
#include <initializer_list>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#include <pagmo/exceptions.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/topologies/base_bgl_topology.hpp>

using namespace pagmo;

using bbt = base_bgl_topology;

TEST(base_bgl_topology, basic_test)
{
    bbt t0;
    EXPECT_TRUE(t0.num_vertices() == 0u);

    t0.add_vertex();
    EXPECT_TRUE(t0.num_vertices() == 1u);
    EXPECT_TRUE(t0.get_connections(0).first.empty());
    EXPECT_TRUE(t0.get_connections(0).second.empty());

    t0.add_vertex();
    t0.add_vertex();
    t0.add_vertex();
    EXPECT_TRUE(t0.num_vertices() == 4u);
    EXPECT_TRUE(t0.get_connections(0).first.empty());
    EXPECT_TRUE(t0.get_connections(0).second.empty());
    EXPECT_TRUE(t0.get_connections(1).first.empty());
    EXPECT_TRUE(t0.get_connections(1).second.empty());
    EXPECT_TRUE(t0.get_connections(2).first.empty());
    EXPECT_TRUE(t0.get_connections(2).second.empty());
    EXPECT_TRUE(t0.get_connections(3).first.empty());
    EXPECT_TRUE(t0.get_connections(3).second.empty());

    t0.add_edge(0, 1);
    t0.add_edge(0, 2);
    EXPECT_TRUE(t0.are_adjacent(0, 1));
    EXPECT_TRUE(t0.are_adjacent(0, 2));
    EXPECT_TRUE(!t0.are_adjacent(1, 0));
    EXPECT_TRUE(!t0.are_adjacent(2, 0));

    t0.add_edge(1, 0);
    t0.add_edge(2, 0);
    EXPECT_TRUE(t0.get_edge_weight(1, 0) == 1.);
    EXPECT_TRUE(t0.get_edge_weight(2, 0) == 1.);

    auto conns = t0.get_connections(0);
    using c_vec = decltype(conns.first);
    using w_vec = decltype(conns.second);

    EXPECT_TRUE((conns.first == c_vec{1, 2} || conns.first == c_vec{2, 1}));
    EXPECT_TRUE((conns.second == w_vec{1., 1.}));

    conns = t0.get_connections(1);

    EXPECT_TRUE((conns.first == c_vec{0}));
    EXPECT_TRUE((conns.second == w_vec{1.}));

    conns = t0.get_connections(2);

    EXPECT_TRUE((conns.first == c_vec{0}));
    EXPECT_TRUE((conns.second == w_vec{1.}));

    t0.remove_edge(0, 2);
    EXPECT_TRUE(t0.get_connections(2).first.empty());
    EXPECT_TRUE(t0.get_connections(2).second.empty());

    t0.set_weight(0, 1, .5);
    EXPECT_TRUE(t0.get_edge_weight(0, 1) == .5);

    conns = t0.get_connections(1);

    EXPECT_TRUE((conns.first == c_vec{0}));
    EXPECT_TRUE((conns.second == w_vec{.5}));

    t0.set_all_weights(.25);
    EXPECT_TRUE(t0.get_connections(0).second.size() == 2u);
    EXPECT_TRUE(t0.get_connections(0).second[0] == .25);
    EXPECT_TRUE(t0.get_connections(0).second[1] == .25);

    // Test copy/move.
    auto t1(t0);
    EXPECT_TRUE(t1.get_connections(0).second.size() == 2u);
    EXPECT_TRUE(t1.get_connections(0).second[0] == .25);
    EXPECT_TRUE(t1.get_connections(0).second[1] == .25);

    auto t2(std::move(t1));
    EXPECT_TRUE(t2.get_connections(0).second.size() == 2u);
    EXPECT_TRUE(t2.get_connections(0).second[0] == .25);
    EXPECT_TRUE(t2.get_connections(0).second[1] == .25);

    bbt t3;
    t3 = t2;
    EXPECT_TRUE(t3.get_connections(0).second.size() == 2u);
    EXPECT_TRUE(t3.get_connections(0).second[0] == .25);
    EXPECT_TRUE(t3.get_connections(0).second[1] == .25);

    bbt t4;
    t4 = std::move(t3);
    EXPECT_TRUE(t4.get_connections(0).second.size() == 2u);
    EXPECT_TRUE(t4.get_connections(0).second[0] == .25);
    EXPECT_TRUE(t4.get_connections(0).second[1] == .25);

    const auto str = t4.get_extra_info();
    EXPECT_TRUE(str.contains("Number of vertices: 4"));
    EXPECT_TRUE(str.contains("Number of edges: 3"));
}

TEST(base_bgl_topology, error_handling)
{
    bbt t0;

    EXPECT_THROW(t0.are_adjacent(0, 1), index_error);

    t0.add_vertex();
    t0.add_vertex();
    t0.add_vertex();

    EXPECT_THROW(t0.get_connections(42), index_error);

    EXPECT_THROW(t0.add_edge(4, 5), index_error);

    t0.add_edge(0, 2);
    EXPECT_THROW(t0.add_edge(0, 2), index_error);

    t0.remove_edge(0, 2);
    EXPECT_THROW(t0.remove_edge(0, 2), index_error);

    t0.add_edge(0, 2);
    t0.set_weight(0, 2, .2);
    EXPECT_THROW(t0.set_weight(0, 2, -1.), invalid_value_error);
    EXPECT_THROW(t0.set_all_weights(-1.), invalid_value_error);
    EXPECT_THROW(t0.set_weight(0, 2, std::numeric_limits<double>::infinity()), invalid_value_error);
    EXPECT_THROW(t0.set_all_weights(std::numeric_limits<double>::infinity()), invalid_value_error);
    EXPECT_THROW(t0.set_weight(0, 1, .2), index_error);

    EXPECT_THROW(t0.get_edge_weight(0, 1), index_error);
    EXPECT_THROW(t0.get_edge_weight(0, 10), index_error);
    EXPECT_THROW(t0.get_edge_weight(11, 10), index_error);
}

TEST(base_bgl_topology, s11n_test)
{
    bbt t0;
    t0.add_vertex();
    t0.add_vertex();
    t0.add_vertex();
    t0.add_vertex();
    t0.add_edge(0, 1);
    t0.add_edge(0, 2);
    t0.add_edge(1, 0);
    t0.set_weight(0, 1, .5);

    // Minimal serialization test.
    {
        std::stringstream ss;
        {
            cereal::BinaryOutputArchive oarchive(ss);
            oarchive(t0);
        }
        bbt t1;
        {
            cereal::BinaryInputArchive iarchive(ss);
            iarchive(t1);
        }
        EXPECT_TRUE(t1.num_vertices() == 4u);
        EXPECT_TRUE(t1.are_adjacent(0, 1));
        EXPECT_TRUE(t1.are_adjacent(0, 2));
        EXPECT_TRUE(t1.are_adjacent(1, 0));
        EXPECT_TRUE(!t1.are_adjacent(2, 0));
        EXPECT_TRUE(t1.get_connections(1).second.size() == 1u);
        EXPECT_TRUE(t1.get_connections(1).second[0] == .5);
    }
}

TEST(base_bgl_topology, thread_torture_test)
{
    std::atomic<int> barrier(0), failures(0);

    bbt t0;
    t0.add_vertex();
    t0.add_vertex();
    t0.add_vertex();
    t0.add_vertex();
    t0.add_edge(0, 1);
    t0.add_edge(0, 2);
    t0.add_edge(1, 0);
    t0.set_weight(0, 1, .5);

    std::vector<std::thread> threads;
    for (auto i = 0; i < 10; ++i) {
        threads.emplace_back([&barrier, &failures, &t0]() {
            ++barrier;
            while (barrier.load() != 10) {
            }

            for (int j = 0; j < 100; ++j) {
                auto t1(t0);
                bbt t2;
                t2 = t0;

                t0.add_vertex();
                failures += t0.num_vertices() < 4u;
                t0.add_vertex();
                failures += !t0.are_adjacent(0, 1);
                t0.add_vertex();
                failures += t0.get_connections(0).first.size() == 0u;
                t0.add_vertex();

                try {
                    t0.add_edge(0u, 4u);
                    t0.set_weight(0u, 4u, .3);
                    t0.remove_edge(0u, 4u);
                } catch (const pagmo_exception &) {
                }

                t0.set_all_weights(.1);

                failures += t0.get_extra_info().empty();

                t0 = std::move(t2);
            }
        });
    }

    for (auto &t : threads) {
        t.join();
    }

    EXPECT_TRUE(failures.load() == 0);
}

TEST(base_bgl_topology, to_graph_test)
{
    bbt t0;

    auto b = t0.to_graph();

    EXPECT_TRUE(b.vertex_count() == 0u);

    t0.add_vertex();
    t0.add_vertex();
    t0.add_vertex();

    b = t0.to_graph();
    EXPECT_TRUE(b.vertex_count() == 3u);
    EXPECT_TRUE(b.get_neighbors(0).empty());
    EXPECT_TRUE(b.get_neighbors(1).empty());
    EXPECT_TRUE(b.get_neighbors(2).empty());

    t0.add_edge(0, 1, .25);
    t0.add_edge(1, 2, 1);
    b = t0.to_graph();
    EXPECT_TRUE(b.vertex_count() == 3u);
    EXPECT_TRUE(b.get_neighbors(0).size() == 1u);
    for (const auto nid : b.get_neighbors(0)) {
        EXPECT_TRUE(b.has_edge(0, nid));
        EXPECT_TRUE(b.get_edge(0, nid) == .25);
    }
    EXPECT_TRUE(b.get_neighbors(1).size() == 1u);
    for (const auto nid : b.get_neighbors(1)) {
        EXPECT_TRUE(b.has_edge(1, nid));
        EXPECT_TRUE(b.get_edge(1, nid) == 1.);
    }
    EXPECT_TRUE(b.get_neighbors(2).empty());
}

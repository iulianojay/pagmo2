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
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <pagmo/exceptions.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/topologies/free_form.hpp>
#include <pagmo/topologies/ring.hpp>
#include <pagmo/topology.hpp>

using namespace pagmo;

TEST(free_form, basic_test)
{
    EXPECT_TRUE(IsUdTopology<free_form>);

    free_form f0;
    EXPECT_TRUE(f0.num_vertices() == 0u);

    f0.push_back();
    f0.push_back();
    f0.push_back();

    EXPECT_TRUE(f0.num_vertices() == 3u);

    EXPECT_TRUE(f0.get_connections(0).first.empty());
    EXPECT_TRUE(f0.get_connections(0).second.empty());

    EXPECT_TRUE(f0.get_connections(1).first.empty());
    EXPECT_TRUE(f0.get_connections(1).second.empty());

    EXPECT_TRUE(f0.get_connections(2).first.empty());
    EXPECT_TRUE(f0.get_connections(2).second.empty());

    EXPECT_TRUE(!f0.are_adjacent(0, 1));
    EXPECT_TRUE(!f0.are_adjacent(0, 2));
    EXPECT_TRUE(!f0.are_adjacent(1, 2));

    // Copy ctor.
    auto f1(f0);

    EXPECT_TRUE(f1.num_vertices() == 3u);

    EXPECT_TRUE(f1.get_connections(0).first.empty());
    EXPECT_TRUE(f1.get_connections(0).second.empty());

    EXPECT_TRUE(f1.get_connections(1).first.empty());
    EXPECT_TRUE(f1.get_connections(1).second.empty());

    EXPECT_TRUE(f1.get_connections(2).first.empty());
    EXPECT_TRUE(f1.get_connections(2).second.empty());

    EXPECT_TRUE(!f1.are_adjacent(0, 1));
    EXPECT_TRUE(!f1.are_adjacent(0, 2));
    EXPECT_TRUE(!f1.are_adjacent(1, 2));

    // Move ctor.
    auto f2(std::move(f1));

    EXPECT_TRUE(f2.num_vertices() == 3u);

    EXPECT_TRUE(f2.get_connections(0).first.empty());
    EXPECT_TRUE(f2.get_connections(0).second.empty());

    EXPECT_TRUE(f2.get_connections(1).first.empty());
    EXPECT_TRUE(f2.get_connections(1).second.empty());

    EXPECT_TRUE(f2.get_connections(2).first.empty());
    EXPECT_TRUE(f2.get_connections(2).second.empty());

    EXPECT_TRUE(!f2.are_adjacent(0, 1));
    EXPECT_TRUE(!f2.are_adjacent(0, 2));
    EXPECT_TRUE(!f2.are_adjacent(1, 2));

    EXPECT_TRUE(f0.get_name() == "Free form");
    EXPECT_TRUE(topology{f0}.get_name() == "Free form");

    // Minimal serialization test.
    {
        topology t0(f0);
        std::stringstream ss;
        {
            cereal::BinaryOutputArchive oarchive(ss);
            oarchive(t0);
        }
        topology t1;
        EXPECT_TRUE(!t1.is<free_form>());
        {
            cereal::BinaryInputArchive iarchive(ss);
            iarchive(t1);
        }
        EXPECT_TRUE(t1.is<free_form>());
        EXPECT_TRUE(t1.extract<free_form>()->num_vertices() == 3u);

        EXPECT_TRUE(t1.extract<free_form>()->get_connections(0).first.empty());
        EXPECT_TRUE(t1.extract<free_form>()->get_connections(0).second.empty());

        EXPECT_TRUE(t1.extract<free_form>()->get_connections(1).first.empty());
        EXPECT_TRUE(t1.extract<free_form>()->get_connections(1).second.empty());

        EXPECT_TRUE(t1.extract<free_form>()->get_connections(2).first.empty());
        EXPECT_TRUE(t1.extract<free_form>()->get_connections(2).second.empty());
    }

    // Example of cout printing for free_form.
    std::cout << topology{f0}.get_extra_info() << '\n';

    // Add a couple of edges.
    f0.add_edge(0, 1, .5);
    f0.add_edge(2, 0);

    EXPECT_TRUE(f0.are_adjacent(0, 1));
    EXPECT_TRUE(f0.get_connections(1).second[0] == .5);
    EXPECT_TRUE(!f0.are_adjacent(1, 0));
    EXPECT_TRUE(!f0.are_adjacent(0, 2));
    EXPECT_TRUE(f0.are_adjacent(2, 0));
    EXPECT_TRUE(f0.get_connections(0).second[0] == 1);
    EXPECT_TRUE(!f0.are_adjacent(1, 2));

    std::cout << topology{f0}.get_extra_info() << '\n';
}

TEST(free_form, graph_ctor)
{
    ring r0{100, .25};
    free_form f0{r0.to_graph()};

    EXPECT_TRUE(f0.num_vertices() == 100u);
    EXPECT_TRUE(f0.are_adjacent(0, 1));
    EXPECT_TRUE(f0.are_adjacent(1, 0));
    EXPECT_TRUE(f0.are_adjacent(0, 99));
    EXPECT_TRUE(f0.are_adjacent(99, 0));
    EXPECT_TRUE(!f0.are_adjacent(0, 2));
    EXPECT_TRUE(!f0.are_adjacent(2, 0));

    for (auto i = 0u; i < 100u; ++i) {
        auto c = f0.get_connections(i);

        EXPECT_TRUE(c.first.size() == 2u);
        EXPECT_TRUE(c.second.size() == 2u);
        EXPECT_TRUE(std::all_of(c.second.begin(), c.second.end(), [](double w) { return w == .25; }));
    }

    // Test error throwing with invalid weights.
    graph_t bogus;
    bogus.add_vertex(0);
    bogus.add_vertex(0);
    bogus.add_vertex(0);

    bogus.add_edge(0, 1, 0.);
    bogus.add_edge(1, 2, 2.); // invalid: weight > 1

    auto trigger = [&bogus]() { free_form fobus(bogus); };

    EXPECT_THROW(trigger(), invalid_value_error);

    bogus.remove_edge(1, 2);
    bogus.add_edge(1, 2, -1.); // invalid: weight < 0

    EXPECT_THROW(trigger(), invalid_value_error);

    bogus.remove_edge(1, 2);
    bogus.add_edge(1, 2, std::numeric_limits<double>::quiet_NaN()); // invalid: NaN

    EXPECT_THROW(trigger(), invalid_value_error);
}

TEST(free_form, udt_ctor)
{
    ring r0{100, .25};
    free_form f0{r0};

    EXPECT_TRUE(f0.num_vertices() == 100u);
    EXPECT_TRUE(f0.are_adjacent(0, 1));
    EXPECT_TRUE(f0.are_adjacent(1, 0));
    EXPECT_TRUE(f0.are_adjacent(0, 99));
    EXPECT_TRUE(f0.are_adjacent(99, 0));
    EXPECT_TRUE(!f0.are_adjacent(0, 2));
    EXPECT_TRUE(!f0.are_adjacent(2, 0));

    for (auto i = 0u; i < 100u; ++i) {
        auto c = f0.get_connections(i);

        EXPECT_TRUE(c.first.size() == 2u);
        EXPECT_TRUE(c.second.size() == 2u);
        EXPECT_TRUE(std::all_of(c.second.begin(), c.second.end(), [](double w) { return w == .25; }));
    }
}

TEST(free_form, topology_ctor)
{
    ring r0{100, .25};
    free_form f0{topology{r0}};

    EXPECT_TRUE(f0.num_vertices() == 100u);
    EXPECT_TRUE(f0.are_adjacent(0, 1));
    EXPECT_TRUE(f0.are_adjacent(1, 0));
    EXPECT_TRUE(f0.are_adjacent(0, 99));
    EXPECT_TRUE(f0.are_adjacent(99, 0));
    EXPECT_TRUE(!f0.are_adjacent(0, 2));
    EXPECT_TRUE(!f0.are_adjacent(2, 0));

    for (auto i = 0u; i < 100u; ++i) {
        auto c = f0.get_connections(i);

        EXPECT_TRUE(c.first.size() == 2u);
        EXPECT_TRUE(c.second.size() == 2u);
        EXPECT_TRUE(std::all_of(c.second.begin(), c.second.end(), [](double w) { return w == .25; }));
    }
}

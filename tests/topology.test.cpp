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

#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

#include <pagmo/detail/type_name.hpp>
#include <pagmo/exceptions.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/topologies/ring.hpp>
#include <pagmo/topologies/unconnected.hpp>
#include <pagmo/topology.hpp>
#include <pagmo/types.hpp>

using namespace pagmo;

struct gc00 {
    std::pair<std::vector<std::size_t>, vector_double> get_connections(std::size_t) const;
};

struct ngc00 {
    void get_connections(std::size_t) const;
};

struct ngc01 {
    std::pair<std::vector<std::size_t>, vector_double> get_connections(std::size_t);
};

struct pb00 {
    void push_back();
};

struct npb00 {
};

struct npb01 {
    int push_back();
};

struct with_to_graph {
    graph_t to_graph() const;
};

struct udt00 {
    std::pair<std::vector<std::size_t>, vector_double> get_connections(std::size_t) const
    {
        return {{0, 1, 2}, {0.1, 0.2, 0.3}};
    }
    void push_back()
    {
        ++n_pushed;
    }
    std::string get_name() const
    {
        return "udt00";
    }
    template <typename Archive>
    void serialize(Archive &ar, unsigned)
    {
        ar & n_pushed;
    }
    int n_pushed = 0;
};

struct udt00a {
    std::pair<std::vector<std::size_t>, vector_double> get_connections(std::size_t) const
    {
        return {{0, 1, 2}, {0.1, 0.2, 0.3}};
    }
    void push_back() {}
};

PAGMO_S11N_TOPOLOGY_EXPORT(udt00)

struct udt01 {
    std::pair<std::vector<std::size_t>, vector_double> get_connections(std::size_t) const
    {
        return {{3, 4, 5}, {0.1, 0.2}};
    }
    void push_back()
    {
        ++n_pushed;
    }
    std::string get_extra_info() const
    {
        return "hello";
    }
    graph_t to_graph() const
    {
        return graph_t{};
    }
    int n_pushed = 0;
};

TEST(topology_test, topology_type_traits_test)
{
    EXPECT_TRUE(!HasGetConnections<void>);
    EXPECT_TRUE(HasGetConnections<gc00>);
    EXPECT_TRUE(!HasGetConnections<ngc00>);
    EXPECT_TRUE(!HasGetConnections<ngc01>);

    EXPECT_TRUE(!HasPushBack<void>);
    EXPECT_TRUE(HasPushBack<pb00>);
    EXPECT_TRUE(!HasPushBack<npb00>);
    EXPECT_TRUE(!HasPushBack<npb01>);

    EXPECT_TRUE(!IsUdTopology<void>);
    EXPECT_TRUE(IsUdTopology<udt00>);
    EXPECT_TRUE(!IsUdTopology<gc00>);
    EXPECT_TRUE(!IsUdTopology<pb00>);
}

TEST(topology_test, topology_basic_tests)
{
    topology def00;
    EXPECT_TRUE(def00.is<unconnected>());

    EXPECT_TRUE((!std::is_constructible<topology, int>::value));
    EXPECT_TRUE((!std::is_constructible<topology, pb00>::value));

    topology t0{udt00{}}, t1{udt01{}};

    EXPECT_TRUE(t0.is_valid());
    EXPECT_TRUE(t0.is<udt00>());
    EXPECT_TRUE(!t0.is<udt01>());
    EXPECT_TRUE(t0.extract<udt00>() != nullptr);
    EXPECT_TRUE(static_cast<const topology &>(t0).extract<udt00>() != nullptr);
    EXPECT_TRUE(t0.extract<udt01>() == nullptr);
    EXPECT_TRUE(static_cast<const topology &>(t0).extract<udt01>() == nullptr);
    EXPECT_TRUE(t0.get_name() == "udt00");
    EXPECT_TRUE(topology{udt00a{}}.get_name() == detail::type_name<udt00a>());
    EXPECT_TRUE(t0.get_extra_info().empty());

    t0.push_back();
    t0.push_back();

    EXPECT_TRUE(t0.extract<udt00>()->n_pushed == 2);

    // Copy construction.
    auto t3(t0);
    EXPECT_TRUE(t3.is_valid());
    EXPECT_TRUE(t3.is<udt00>());
    EXPECT_TRUE(t3.extract<udt00>()->n_pushed == 2);
    EXPECT_TRUE(static_cast<const topology &>(t3).extract<udt00>()->n_pushed == 2);
    EXPECT_TRUE(t3.get_name() == "udt00");
    EXPECT_TRUE(t3.get_extra_info().empty());

    // Copy assignment.
    topology t4;
    t4 = t3;
    EXPECT_TRUE(t4.is_valid());
    EXPECT_TRUE(t4.is<udt00>());
    EXPECT_TRUE(t4.extract<udt00>()->n_pushed == 2);
    EXPECT_TRUE(static_cast<const topology &>(t4).extract<udt00>()->n_pushed == 2);
    EXPECT_TRUE(t4.get_name() == "udt00");

    // Move construction.
    auto t5(std::move(t4));
    EXPECT_TRUE(!t4.is_valid());
    EXPECT_TRUE(t5.is_valid());
    EXPECT_TRUE(t5.is<udt00>());
    EXPECT_TRUE(t5.extract<udt00>()->n_pushed == 2);
    EXPECT_TRUE(t5.get_name() == "udt00");
    EXPECT_TRUE(t5.get_extra_info().empty());

    // Move assignment.
    topology t6;
    t6 = std::move(t5);
    EXPECT_TRUE(!t5.is_valid());
    EXPECT_TRUE(t6.is_valid());
    EXPECT_TRUE(t6.is<udt00>());
    EXPECT_TRUE(t6.extract<udt00>()->n_pushed == 2);
    EXPECT_TRUE(t6.get_name() == "udt00");

    // Generic assignment.
    EXPECT_TRUE((!std::is_assignable<topology &, int>::value));
    t6 = udt01{};
    EXPECT_TRUE(t6.is_valid());
    EXPECT_TRUE(t6.is<udt01>());
    EXPECT_TRUE(t6.extract<udt01>()->n_pushed == 0);
    EXPECT_TRUE(t6.get_extra_info() == "hello");
}

// Bad connections.
struct bc00 : udt00 {
    // Inconsistent vector sizes.
    std::pair<std::vector<std::size_t>, vector_double> get_connections(std::size_t) const
    {
        return {{0, 1}, {0.1, 0.2, 0.3}};
    }
};

struct bc01 : udt00 {
    // Non-finite weight.
    std::pair<std::vector<std::size_t>, vector_double> get_connections(std::size_t) const
    {
        return {{0, 1}, {0.1, std::numeric_limits<double>::infinity()}};
    }
};

struct bc02 : udt00 {
    // Weight outside the probability range.
    std::pair<std::vector<std::size_t>, vector_double> get_connections(std::size_t) const
    {
        return {{0, 1}, {0.1, 2.}};
    }
};

TEST(topology_test, topology_get_connections_test)
{
    topology t0{udt00{}};
    EXPECT_TRUE(t0.get_connections(0) == t0.get_connections(1));
    EXPECT_TRUE((t0.get_connections(0).first == std::vector<std::size_t>{0, 1, 2}));
    EXPECT_TRUE((t0.get_connections(0).second == std::vector<double>{.1, .2, .3}));

    t0 = bc00{};

    EXPECT_THROW(t0.get_connections(0), dimension_mismatch_error);

    t0 = bc01{};

    EXPECT_THROW(t0.get_connections(0), invalid_value_error);

    t0 = bc02{};

    EXPECT_THROW(t0.get_connections(0), invalid_value_error);
}

TEST(topology_test, topology_s11n_test)
{
    topology t0{udt00{}};
    t0.push_back();
    t0.push_back();

    std::stringstream ss;
    {
        cereal::BinaryOutputArchive oarchive(ss);
        oarchive(t0);
    }
    topology t1;
    EXPECT_TRUE(!t1.is<udt00>());
    {
        cereal::BinaryInputArchive iarchive(ss);
        iarchive(t1);
    }

    EXPECT_TRUE(t1.is<udt00>());
    EXPECT_TRUE(t1.get_name() == "udt00");
    EXPECT_TRUE(t1.extract<udt00>()->n_pushed == 2);
}

TEST(topology_test, topology_stream_test)
{
    {
        topology t0{udt01{}};

        std::ostringstream oss;

        oss << t0;

        auto str = oss.str();

        EXPECT_TRUE(str.contains("Topology name:"));
        EXPECT_TRUE(str.contains("hello"));
    }

    {
        topology t0{udt00{}};

        std::ostringstream oss;

        oss << t0;

        auto str = oss.str();

        EXPECT_TRUE(str.contains("Topology name: udt00"));
    }

    std::cout << topology{} << '\n';
}

TEST(topology_test, topology_push_back_n_test)
{
    topology t0{ring{}};

    t0.push_back(0);

    EXPECT_TRUE(t0.extract<ring>()->num_vertices() == 0u);

    t0.push_back(2);

    EXPECT_TRUE(t0.extract<ring>()->num_vertices() == 2u);
    EXPECT_TRUE(t0.get_connections(0).first.size() == 1u);
    EXPECT_TRUE(t0.get_connections(0).first[0] == 1u);
    EXPECT_TRUE(t0.get_connections(1).first.size() == 1u);
    EXPECT_TRUE(t0.get_connections(1).first[0] == 0u);

    t0.push_back(5);

    EXPECT_TRUE(t0.extract<ring>()->num_vertices() == 7u);
}

TEST(topology_test, topology_to_graph_test)
{
    EXPECT_TRUE(!HasToGraph<gc00>);
    EXPECT_TRUE(HasToGraph<with_to_graph>);

    EXPECT_THROW(topology{udt00{}}.to_graph(), not_implemented_error);

    EXPECT_TRUE(topology{udt01{}}.to_graph().vertex_count() == 0);
}

TEST(topology_test, type_index)
{
    topology p0;
    EXPECT_TRUE(p0.get_type_index() == std::type_index(typeid(unconnected)));
    p0 = topology{udt00a{}};
    EXPECT_TRUE(p0.get_type_index() == std::type_index(typeid(udt00a)));
}

TEST(topology_test, get_ptr)
{
    topology p0;
    EXPECT_TRUE(p0.get_ptr() == p0.extract<unconnected>());
    EXPECT_TRUE(static_cast<const topology &>(p0).get_ptr()
                == static_cast<const topology &>(p0).extract<unconnected>());
    p0 = topology{udt00a{}};
    EXPECT_TRUE(p0.get_ptr() == p0.extract<udt00a>());
    EXPECT_TRUE(static_cast<const topology &>(p0).get_ptr() == static_cast<const topology &>(p0).extract<udt00a>());
}

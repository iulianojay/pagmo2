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

#include <sstream>

#include <pagmo/exceptions.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/topologies/ring.hpp>
#include <pagmo/topologies/unconnected.hpp>
#include <pagmo/topology.hpp>

using namespace pagmo;

TEST(unconnected, basic_test)
{
    unconnected r0;

    EXPECT_TRUE(r0.get_connections(0).first.empty());
    EXPECT_TRUE(r0.get_connections(0).second.empty());

    r0.push_back();

    EXPECT_TRUE(r0.get_connections(1).first.empty());
    EXPECT_TRUE(r0.get_connections(1).second.empty());

    // Minimal serialization test.
    {
        topology t0(r0);
        std::stringstream ss;
        {
            cereal::BinaryOutputArchive oarchive(ss);
            oarchive(t0);
        }
        topology t1(ring{});
        EXPECT_TRUE(!t1.is<unconnected>());
        {
            cereal::BinaryInputArchive iarchive(ss);
            iarchive(t1);
        }
        EXPECT_TRUE(t1.is<unconnected>());
    }
}

TEST(unconnected, to_graph_test)
{
    EXPECT_TRUE(!HasToGraph<unconnected>);

    EXPECT_THROW(topology{unconnected{}}.to_graph(), not_implemented_error);
}

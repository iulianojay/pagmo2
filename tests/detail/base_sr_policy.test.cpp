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

#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

#include <pagmo/detail/base_sr_policy.hpp>
#include <pagmo/exceptions.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/types.hpp>

using namespace pagmo;

using bsrp = detail::base_sr_policy;

TEST(base_sr_policy, basic_test)
{
    bsrp b0(0);
    EXPECT_TRUE(b0.get_migr_rate().index() == 0);
    EXPECT_TRUE(std::get<pop_size_t>(b0.get_migr_rate()) == 0);

    b0 = bsrp(4u);
    EXPECT_TRUE(b0.get_migr_rate().index() == 0);
    EXPECT_TRUE(std::get<pop_size_t>(b0.get_migr_rate()) == 4);

    b0 = bsrp(.1);
    EXPECT_TRUE(b0.get_migr_rate().index() == 1);
    EXPECT_TRUE(std::get<double>(b0.get_migr_rate()) == .1);

    b0 = bsrp(0.l);
    EXPECT_TRUE(b0.get_migr_rate().index() == 1);
    EXPECT_TRUE(std::get<double>(b0.get_migr_rate()) == 0.);

    b0 = bsrp(1.f);
    EXPECT_TRUE(b0.get_migr_rate().index() == 1);
    EXPECT_TRUE(std::get<double>(b0.get_migr_rate()) == 1.);

    EXPECT_TRUE((!std::is_constructible<bsrp, const std::string &>::value));

    // Minimal serialization test.
    {
        std::stringstream ss;
        {
            cereal::BinaryOutputArchive oarchive(ss);
            oarchive(b0);
        }
        bsrp b1(0);
        {
            cereal::BinaryInputArchive iarchive(ss);
            iarchive(b1);
        }
        EXPECT_TRUE(b1.get_migr_rate().index() == 1);
        EXPECT_TRUE(std::get<double>(b1.get_migr_rate()) == 1.);
    }

    // Error handling.
    EXPECT_THROW(b0 = bsrp(-1.), policy_config_error);
    EXPECT_THROW(b0 = bsrp(2.), policy_config_error);
    EXPECT_THROW(b0 = bsrp(std::numeric_limits<double>::infinity()), policy_config_error);
    EXPECT_THROW(b0 = bsrp(-1), std::overflow_error);
}

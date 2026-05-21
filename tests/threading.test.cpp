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

#include <iostream>
#include <sstream>

#include <pagmo/threading.hpp>

using namespace pagmo;

TEST(threading_test, threading_test)
{
    // Check the ordering of the safety levels.
    EXPECT_TRUE(thread_safety::none < thread_safety::basic);
    EXPECT_TRUE(thread_safety::basic < thread_safety::constant);
    EXPECT_TRUE(thread_safety::none <= thread_safety::basic);
    EXPECT_TRUE(thread_safety::basic <= thread_safety::constant);
    EXPECT_TRUE(thread_safety::none <= thread_safety::none);
    EXPECT_TRUE(thread_safety::basic <= thread_safety::basic);
    EXPECT_TRUE(thread_safety::constant <= thread_safety::constant);
    EXPECT_TRUE(thread_safety::basic > thread_safety::none);
    EXPECT_TRUE(thread_safety::constant > thread_safety::basic);
    EXPECT_TRUE(thread_safety::basic >= thread_safety::none);
    EXPECT_TRUE(thread_safety::constant >= thread_safety::basic);
    EXPECT_TRUE(thread_safety::none >= thread_safety::none);
    EXPECT_TRUE(thread_safety::basic >= thread_safety::basic);
    EXPECT_TRUE(thread_safety::constant >= thread_safety::constant);

    // Test the streaming operator.
    std::ostringstream oss;
    oss << thread_safety::none;
    EXPECT_EQ("none", oss.str());
    oss.str("");
    oss << thread_safety::basic;
    EXPECT_EQ("basic", oss.str());
    oss.str("");
    oss << thread_safety::constant;
    EXPECT_EQ("constant", oss.str());
    oss.str("");
    oss << static_cast<thread_safety>(100);
    EXPECT_EQ("unknown value", oss.str());
}

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

#include <pagmo/io.hpp>

#include <initializer_list>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <pagmo/threading.hpp>

using namespace pagmo;

TEST(io_test, stream_print_test_00)
{
    // A few simple tests.
    std::ostringstream ss1, ss2;
    stream(ss1, 1, 2, 3);
    ss2 << 1 << 2 << 3;
    EXPECT_EQ(ss1.str(), ss2.str());
    ss1.str("");
    ss2.str("");
    stream(ss1, "Hello ", std::string(" world"));
    ss2 << "Hello " << std::string(" world");
    EXPECT_EQ(ss1.str(), ss2.str());
    ss1.str("");
    ss2.str("");
    // Try with floating-point too.
    stream(ss1, 1.234);
    ss2 << 1.234;
    EXPECT_EQ(ss1.str(), ss2.str());
    ss1.str("");
    ss2.str("");
    // Custom precision.
    ss1 << std::setprecision(10);
    ss2 << std::setprecision(10);
    stream(ss1, 1.234);
    ss2 << 1.234;
    EXPECT_EQ(ss1.str(), ss2.str());
    ss1.str("");
    ss2.str("");
    // Special handling of bool.
    stream(ss1, true, ' ', false);
    EXPECT_EQ(ss1.str(), "true false");
    ss1.str("");
    // Vectors.
    // Empty vector.
    stream(ss1, std::vector<int>{});
    EXPECT_EQ(ss1.str(), "[]");
    ss1.str("");
    // Single element.
    stream(ss1, std::vector<int>{1});
    ss2 << "[" << 1 << "]";
    EXPECT_EQ(ss1.str(), ss2.str());
    ss1.str("");
    ss2.str("");
    // Multiple elements.
    stream(ss1, std::vector<int>{1, 2, 3});
    ss2 << "[" << 1 << ", " << 2 << ", " << 3 << "]";
    EXPECT_EQ(ss1.str(), ss2.str());
    ss1.str("");
    ss2.str("");
    // Vector equal to the print limit.
    stream(ss1, std::vector<int>{1, 2, 3, 4, 5});
    ss2 << "[" << 1 << ", " << 2 << ", " << 3 << ", " << 4 << ", " << 5 << "]";
    EXPECT_EQ(ss1.str(), ss2.str());
    ss1.str("");
    ss2.str("");
    // Vector larger than the print limit.
    stream(ss1, std::vector<int>{1, 2, 3, 4, 5, 6});
    ss2 << "[" << 1 << ", " << 2 << ", " << 3 << ", " << 4 << ", " << 5 << ", ... ]";
    EXPECT_EQ(ss1.str(), ss2.str());
    // Go for the print as well, yay.
    print(std::vector<int>{1, 2, 3, 4, 5, 6});
    // Thread safety levels.
    ss1.str("");
    stream(ss1, thread_safety::none);
    EXPECT_EQ(ss1.str(), "none");
    ss1.str("");
    stream(ss1, thread_safety::basic);
    EXPECT_EQ(ss1.str(), "basic");
}

TEST(io_test, stream_print_test_01)
{
    // Map.
    using map_t = std::map<int, int>;
    std::stringstream ss;
    stream(ss, map_t{{0, 0}, {1, 1}, {2, 2}});
    print(map_t{{0, 0}, {1, 1}, {2, 2}});
    ss.str("");
    stream(ss, map_t{{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}});
    print(map_t{{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}});
    // Pair.
    using pair_t = std::pair<int, int>;
    ss.str("");
    stream(ss, pair_t{1, 2});
    print(pair_t{1, 2});
}

TEST(io_test, stream_table_test)
{
    detail::table t({"a", "b", "c"});
    EXPECT_THROW(t.add_row(), dimension_mismatch_error);
}
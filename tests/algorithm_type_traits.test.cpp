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

#include <utility>

#include <pagmo/algorithm.hpp>
#include <pagmo/population.hpp>

using namespace pagmo;

struct hsv_00 {
};

// The good one
struct hsv_01 {
    void set_verbosity(unsigned);
};

// also good
struct hsv_02 {
    void set_verbosity(unsigned) const;
};

// also good
struct hsv_03 {
    void set_verbosity(int);
};

struct hsv_04 {
    double set_verbosity(unsigned);
};

TEST(algorithm_type_traits, has_set_verbose_test)
{
    EXPECT_TRUE((!HasSetVerbosity<hsv_00>));
    EXPECT_TRUE((HasSetVerbosity<hsv_01>));
    EXPECT_TRUE((HasSetVerbosity<hsv_02>));
    EXPECT_TRUE((HasSetVerbosity<hsv_03>));
    EXPECT_TRUE((!HasSetVerbosity<hsv_04>));
}

struct hev_00 {
};

// The good one
struct hev_01 {
    population evolve(population) const;
};

struct hev_02 {
    population evolve(const population &);
};

struct hev_03 {
    population evolve(population &) const;
};

struct hev_04 {
    double evolve(const population &) const;
};

struct hev_05 {
    population evolve(const double &) const;
};

TEST(algorithm_type_traits, has_evolve_test)
{
    EXPECT_TRUE((!HasEvolve<hev_00>));
    EXPECT_TRUE((HasEvolve<hev_01>));
    EXPECT_TRUE((!HasEvolve<hev_02>));
    EXPECT_TRUE((!HasEvolve<hev_03>));
    EXPECT_TRUE((!HasEvolve<hev_04>));
    EXPECT_TRUE((!HasEvolve<hev_05>));
}

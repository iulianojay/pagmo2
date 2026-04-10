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

// This file is compiled into every test executable.  It calls
// cereal::detail::dynamic_init_dummy_pagmo() via a static initializer, which
// ensures that pagmo/s11n_registrations.cpp (compiled into libpagmo) is NOT
// dead-stripped by the macOS linker and that all cereal polymorphic type
// registrations are executed before any test serialization runs.

#include <pagmo/s11n.hpp>

CEREAL_FORCE_DYNAMIC_INIT(pagmo)

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

#ifndef PAGMO_DETAIL_PAGMO_FWD_HPP
#define PAGMO_DETAIL_PAGMO_FWD_HPP

#include <pagmo/detail/visibility.hpp>

namespace pagmo
{

// Base classes
class PAGMO_DLL_PUBLIC archipelago;
class PAGMO_DLL_PUBLIC algorithm;
class PAGMO_DLL_PUBLIC bfe;
class PAGMO_DLL_PUBLIC island;
class PAGMO_DLL_PUBLIC population;
class PAGMO_DLL_PUBLIC problem;
class PAGMO_DLL_PUBLIC topology;
class PAGMO_DLL_PUBLIC r_policy;
class PAGMO_DLL_PUBLIC s_policy;

// Topologies
class PAGMO_DLL_PUBLIC free_form;

// Algorithms
class PAGMO_DLL_PUBLIC cstrs_self_adaptive;
class PAGMO_DLL_PUBLIC mbh;

// Problems
class PAGMO_DLL_PUBLIC cec2006;
class PAGMO_DLL_PUBLIC cec2009;
class PAGMO_DLL_PUBLIC decompose;
class PAGMO_DLL_PUBLIC translate;
class PAGMO_DLL_PUBLIC unconstrain;

} // namespace pagmo

#endif

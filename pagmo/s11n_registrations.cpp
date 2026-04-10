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

// This translation unit is the single authoritative location for all cereal
// polymorphic type registrations in pagmo.  Centralising them here avoids the
// macOS dyld two-level-namespace problem where each DSO (shared library and
// test executable) would otherwise contain its own copy of
// StaticObject<PolymorphicCasters>, giving each a separate registry.
//
// CEREAL_REGISTER_DYNAMIC_INIT prevents the linker from dead-stripping this TU
// on macOS.  Test executables include CEREAL_FORCE_DYNAMIC_INIT (via
// tests/pagmo_force_dynamic_init.cpp) to ensure these initializers run.

#include <pagmo/config.hpp>
#include <pagmo/s11n.hpp>

// On macOS, cereal's StaticObject<> singletons would otherwise be duplicated: one
// copy in libpagmo.dylib and one in each test executable (two-level namespace).
// The fix is applied in tests/CMakeLists.txt: test executables are linked with
// -Wl,-flat_namespace so that all StaticObject accesses resolve to the same
// (libpagmo-owned) singletons.  CEREAL_REGISTER_DYNAMIC_INIT below ensures that
// this TU is not dead-stripped on macOS.

// ---- Problems ---------------------------------------------------------------
#include <pagmo/problems/ackley.hpp>
#include <pagmo/problems/cec2006.hpp>
#include <pagmo/problems/cec2009.hpp>
#include <pagmo/problems/cec2013.hpp>
#include <pagmo/problems/cec2014.hpp>
#include <pagmo/problems/decompose.hpp>
#include <pagmo/problems/dtlz.hpp>
#include <pagmo/problems/golomb_ruler.hpp>
#include <pagmo/problems/griewank.hpp>
#include <pagmo/problems/hock_schittkowski_71.hpp>
#include <pagmo/problems/inventory.hpp>
#include <pagmo/problems/lennard_jones.hpp>
#include <pagmo/problems/luksan_vlcek1.hpp>
#include <pagmo/problems/minlp_rastrigin.hpp>
#include <pagmo/problems/null_problem.hpp>
#include <pagmo/problems/rastrigin.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/problems/schwefel.hpp>
#include <pagmo/problems/translate.hpp>
#include <pagmo/problems/unconstrain.hpp>
#include <pagmo/problems/wfg.hpp>
#include <pagmo/problems/zdt.hpp>

PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::ackley)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::cec2006)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::cec2009)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::cec2013)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::cec2014)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::decompose)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::dtlz)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::golomb_ruler)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::griewank)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::hock_schittkowski_71)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::inventory)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::lennard_jones)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::luksan_vlcek1)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::minlp_rastrigin)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::null_problem)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::rastrigin)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::rosenbrock)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::schwefel)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::translate)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::unconstrain)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::wfg)
PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::zdt)

// ---- Algorithms -------------------------------------------------------------
#include <pagmo/algorithms/bee_colony.hpp>
#include <pagmo/algorithms/compass_search.hpp>
#include <pagmo/algorithms/cstrs_self_adaptive.hpp>
#include <pagmo/algorithms/de.hpp>
#include <pagmo/algorithms/de1220.hpp>
#include <pagmo/algorithms/gaco.hpp>
#include <pagmo/algorithms/gwo.hpp>
#include <pagmo/algorithms/ihs.hpp>
#include <pagmo/algorithms/maco.hpp>
#include <pagmo/algorithms/mbh.hpp>
#include <pagmo/algorithms/moead.hpp>
#include <pagmo/algorithms/moead_gen.hpp>
#include <pagmo/algorithms/nsga2.hpp>
#include <pagmo/algorithms/nspso.hpp>
#include <pagmo/algorithms/null_algorithm.hpp>
#include <pagmo/algorithms/pso.hpp>
#include <pagmo/algorithms/pso_gen.hpp>
#include <pagmo/algorithms/sade.hpp>
#include <pagmo/algorithms/sea.hpp>
#include <pagmo/algorithms/sga.hpp>
#include <pagmo/algorithms/simulated_annealing.hpp>

PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::bee_colony)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::compass_search)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::cstrs_self_adaptive)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::de)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::de1220)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::gaco)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::gwo)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::ihs)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::maco)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::mbh)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::moead)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::moead_gen)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::nsga2)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::nspso)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::null_algorithm)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::pso)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::pso_gen)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::sade)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::sea)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::sga)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::simulated_annealing)

#if defined(PAGMO_WITH_EIGEN3)
#include <pagmo/algorithms/cmaes.hpp>
#include <pagmo/algorithms/xnes.hpp>
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::cmaes)
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::xnes)
#endif

#if defined(PAGMO_WITH_NLOPT)
#include <pagmo/algorithms/nlopt.hpp>
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::nlopt)
#endif

#if defined(PAGMO_WITH_IPOPT)
#include <pagmo/algorithms/ipopt.hpp>
PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::ipopt)
#endif

// ---- BFEs -------------------------------------------------------------------
#include <pagmo/batch_evaluators/default_bfe.hpp>
#include <pagmo/batch_evaluators/member_bfe.hpp>
#include <pagmo/batch_evaluators/thread_bfe.hpp>

PAGMO_S11N_BFE_IMPLEMENT(pagmo::default_bfe)
PAGMO_S11N_BFE_IMPLEMENT(pagmo::member_bfe)
PAGMO_S11N_BFE_IMPLEMENT(pagmo::thread_bfe)

// ---- Islands ----------------------------------------------------------------
#include <pagmo/islands/thread_island.hpp>
PAGMO_S11N_ISLAND_IMPLEMENT(pagmo::thread_island)

#if defined(PAGMO_WITH_FORK_ISLAND)
#include <pagmo/islands/fork_island.hpp>
PAGMO_S11N_ISLAND_IMPLEMENT(pagmo::fork_island)
#endif

// ---- Topologies -------------------------------------------------------------
#include <pagmo/topologies/free_form.hpp>
#include <pagmo/topologies/fully_connected.hpp>
#include <pagmo/topologies/ring.hpp>
#include <pagmo/topologies/unconnected.hpp>

PAGMO_S11N_TOPOLOGY_IMPLEMENT(pagmo::free_form)
PAGMO_S11N_TOPOLOGY_IMPLEMENT(pagmo::fully_connected)
PAGMO_S11N_TOPOLOGY_IMPLEMENT(pagmo::ring)
PAGMO_S11N_TOPOLOGY_IMPLEMENT(pagmo::unconnected)

// ---- Replacement policies ---------------------------------------------------
#include <pagmo/r_policies/fair_replace.hpp>
PAGMO_S11N_R_POLICY_IMPLEMENT(pagmo::fair_replace)

// ---- Selection policies -----------------------------------------------------
#include <pagmo/s_policies/select_best.hpp>
PAGMO_S11N_S_POLICY_IMPLEMENT(pagmo::select_best)

// Must be in the same TU as all CEREAL_BIND_TO_ARCHIVES calls above.
// Prevents macOS from dead-stripping this translation unit.
CEREAL_REGISTER_DYNAMIC_INIT(pagmo)

# arg1 is the path relative to this directory without extension, e.g. "algorithms/de"
# The basename is used for the target name: test_de
function(ADD_PAGMO_TESTCASE arg1)
    if(PAGMO_TEST_NSPLIT)
        math(EXPR __PAGMO_TEST_NUM "(${_PAGMO_TEST_NUM} + 1) % ${PAGMO_TEST_NSPLIT}")
        set(_PAGMO_TEST_NUM ${__PAGMO_TEST_NUM} PARENT_SCOPE)
    endif()
    if(PAGMO_TEST_NSPLIT AND "${PAGMO_TEST_SPLIT_NUM}" STREQUAL "${_PAGMO_TEST_NUM}")
        return()
    endif()
    get_filename_component(_test_name "${arg1}" NAME)
    # Add 'test_' prefix to avoid conflicts with library targets
    add_executable(test_${_test_name} ${arg1}.test.cpp "${CMAKE_CURRENT_SOURCE_DIR}/pagmo_force_dynamic_init.cpp")
    if(TARGET gtest_main)
        target_link_libraries(test_${_test_name} PRIVATE pagmo gtest_main gtest)
        # Add cereal for tests that include headers with cereal
        if(TARGET cereal::cereal)
            target_link_libraries(test_${_test_name} PRIVATE cereal::cereal)
        elseif(TARGET cereal)
            target_link_libraries(test_${_test_name} PRIVATE cereal)
        endif()
    else()
        message(FATAL_ERROR "Google Test framework target not found")
    endif()
    target_compile_options(test_${_test_name} PRIVATE
        "$<$<CONFIG:Debug>:${PAGMO_CXX_FLAGS_DEBUG}>"
        "$<$<CONFIG:Release>:${PAGMO_CXX_FLAGS_RELEASE}>"
        "$<$<CONFIG:RelWithDebInfo>:${PAGMO_CXX_FLAGS_RELEASE}>"
        "$<$<CONFIG:MinSizeRel>:${PAGMO_CXX_FLAGS_RELEASE}>"
    )
    # Set the minimum C++ standard to C++23
    target_compile_features(test_${_test_name} PRIVATE cxx_std_23)
    target_compile_definitions(test_${_test_name} PRIVATE "_PAGMO_TEST_ROOT_=\"${CMAKE_CURRENT_LIST_DIR}\"")
    set_property(TARGET test_${_test_name} PROPERTY CXX_EXTENSIONS NO)
    set_property(TARGET test_${_test_name} PROPERTY UNITY_BUILD OFF)
    # On macOS, cereal's StaticObject<> singletons would otherwise be duplicated: one copy
    # lives in libpagmo.dylib and another in each test executable (two-level namespace).
    # Polymorphic type registrations that happen in the test exe (e.g. local test types
    # registered via PAGMO_S11N_*_EXPORT) would end up in the test exe's copy and be
    # invisible to the archive code running inside libpagmo.
    # -Wl,-flat_namespace makes the dynamic linker use a single global symbol table, so
    # all StaticObject accesses resolve to the same (libpagmo-owned) singletons.
    if(APPLE AND NOT PAGMO_BUILD_STATIC_LIBRARY)
        target_link_options(test_${_test_name} PRIVATE -Wl,-flat_namespace)
    endif()
    add_test(NAME ${_test_name} COMMAND test_${_test_name})
endfunction()

# Root-level tests
ADD_PAGMO_TESTCASE(algorithm)
ADD_PAGMO_TESTCASE(algorithm_type_traits)
ADD_PAGMO_TESTCASE(archipelago)
ADD_PAGMO_TESTCASE(archipelago_torture_test)
ADD_PAGMO_TESTCASE(bfe)
ADD_PAGMO_TESTCASE(io)
ADD_PAGMO_TESTCASE(island)
ADD_PAGMO_TESTCASE(island_torture)
ADD_PAGMO_TESTCASE(migration_torture_test)
ADD_PAGMO_TESTCASE(population)
ADD_PAGMO_TESTCASE(problem)
ADD_PAGMO_TESTCASE(problem_type_traits)
ADD_PAGMO_TESTCASE(r_policy)
ADD_PAGMO_TESTCASE(rng)
ADD_PAGMO_TESTCASE(rng_serialization)
ADD_PAGMO_TESTCASE(s_policy)
ADD_PAGMO_TESTCASE(threading)
ADD_PAGMO_TESTCASE(topology)
ADD_PAGMO_TESTCASE(type_traits)

# Algorithms
ADD_PAGMO_TESTCASE(algorithms/bee_colony)
ADD_PAGMO_TESTCASE(algorithms/compass_search)
ADD_PAGMO_TESTCASE(algorithms/cstrs_self_adaptive)
ADD_PAGMO_TESTCASE(algorithms/de)
ADD_PAGMO_TESTCASE(algorithms/de1220)
ADD_PAGMO_TESTCASE(algorithms/gaco)
ADD_PAGMO_TESTCASE(algorithms/gwo)
ADD_PAGMO_TESTCASE(algorithms/ihs)
ADD_PAGMO_TESTCASE(algorithms/maco)
ADD_PAGMO_TESTCASE(algorithms/mbh)
ADD_PAGMO_TESTCASE(algorithms/moead)
ADD_PAGMO_TESTCASE(algorithms/moead_gen)
ADD_PAGMO_TESTCASE(algorithms/nsga2)
ADD_PAGMO_TESTCASE(algorithms/nspso)
ADD_PAGMO_TESTCASE(algorithms/pso)
ADD_PAGMO_TESTCASE(algorithms/pso_gen)
ADD_PAGMO_TESTCASE(algorithms/sade)
ADD_PAGMO_TESTCASE(algorithms/sea)
ADD_PAGMO_TESTCASE(algorithms/sga)
ADD_PAGMO_TESTCASE(algorithms/simulated_annealing)

# Batch evaluators
ADD_PAGMO_TESTCASE(batch_evaluators/default_bfe)
ADD_PAGMO_TESTCASE(batch_evaluators/member_bfe)
ADD_PAGMO_TESTCASE(batch_evaluators/thread_bfe)

# Detail
ADD_PAGMO_TESTCASE(detail/base_sr_policy)
ADD_PAGMO_TESTCASE(detail/custom_comparisons)

# Islands
ADD_PAGMO_TESTCASE(islands/thread_island)

# Problems
ADD_PAGMO_TESTCASE(problems/ackley)
ADD_PAGMO_TESTCASE(problems/cec2006)
ADD_PAGMO_TESTCASE(problems/cec2009)
ADD_PAGMO_TESTCASE(problems/cec2013)
ADD_PAGMO_TESTCASE(problems/cec2014)
ADD_PAGMO_TESTCASE(problems/decompose)
ADD_PAGMO_TESTCASE(problems/dtlz)
ADD_PAGMO_TESTCASE(problems/golomb_ruler)
ADD_PAGMO_TESTCASE(problems/griewank)
ADD_PAGMO_TESTCASE(problems/hock_schittkowski_71)
ADD_PAGMO_TESTCASE(problems/inventory)
ADD_PAGMO_TESTCASE(problems/lennard_jones)
ADD_PAGMO_TESTCASE(problems/luksan_vlcek1)
ADD_PAGMO_TESTCASE(problems/minlp_rastrigin)
ADD_PAGMO_TESTCASE(problems/rastrigin)
ADD_PAGMO_TESTCASE(problems/rosenbrock)
ADD_PAGMO_TESTCASE(problems/schwefel)
ADD_PAGMO_TESTCASE(problems/translate)
ADD_PAGMO_TESTCASE(problems/unconstrain)
ADD_PAGMO_TESTCASE(problems/wfg)
ADD_PAGMO_TESTCASE(problems/zdt)

# Replacement policies
ADD_PAGMO_TESTCASE(r_policies/fair_replace)

# Selection policies
ADD_PAGMO_TESTCASE(s_policies/select_best)

# Topologies
ADD_PAGMO_TESTCASE(topologies/base_bgl_topology)
ADD_PAGMO_TESTCASE(topologies/free_form)
ADD_PAGMO_TESTCASE(topologies/fully_connected)
ADD_PAGMO_TESTCASE(topologies/ring)
ADD_PAGMO_TESTCASE(topologies/unconnected)

# Utils
ADD_PAGMO_TESTCASE(utils/constrained)
ADD_PAGMO_TESTCASE(utils/discrepancy)
ADD_PAGMO_TESTCASE(utils/generic)
ADD_PAGMO_TESTCASE(utils/genetic_operators)
ADD_PAGMO_TESTCASE(utils/gradients_and_hessians)
ADD_PAGMO_TESTCASE(utils/hypervolume)
ADD_PAGMO_TESTCASE(utils/multi_objective)

if(PAGMO_WITH_EIGEN3)
    ADD_PAGMO_TESTCASE(algorithms/cmaes)
    ADD_PAGMO_TESTCASE(algorithms/xnes)
    ADD_PAGMO_TESTCASE(detail/eigen3_serialization)
endif()

if(PAGMO_WITH_NLOPT)
    ADD_PAGMO_TESTCASE(algorithms/nlopt)
endif()

if(PAGMO_WITH_IPOPT)
    ADD_PAGMO_TESTCASE(algorithms/ipopt)
endif()

if(PAGMO_WITH_FORK_ISLAND)
    ADD_PAGMO_TESTCASE(islands/fork_island)
endif()

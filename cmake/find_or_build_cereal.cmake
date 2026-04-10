# PaGMO - CMake script to find or build cereal for serialization
# Configures: cereal as a header-only library

# Pinned to a post-1.3.2 master commit that fixes the
# -Wmissing-template-arg-list-after-template-kw error in tuple.hpp
# triggered by LLVM 20 / Apple Clang 17+ (commit d81e2f7, Mar 6 2026).
set(_PAGMO_REQUIRED_CEREAL_VERSION 1.3.2)
set(_PAGMO_CEREAL_GIT_TAG d81e2f7df7b334fee057e53017388d02e555a836)

# Try to find cereal first
find_package(cereal ${_PAGMO_REQUIRED_CEREAL_VERSION} QUIET)

if(NOT cereal_FOUND)
    message(STATUS "cereal not found. Attempting to build from source.")
    
    # Use CPM to download and make cereal available
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)
    
    CPMAddPackage(
        NAME cereal
        VERSION ${_PAGMO_REQUIRED_CEREAL_VERSION}
        GITHUB_REPOSITORY USCiLab/cereal
        GIT_TAG ${_PAGMO_CEREAL_GIT_TAG}
        OPTIONS
            "JUST_INSTALL_CEREAL ON"
            "SKIP_PERFORMANCE_COMPARISON ON"
            "BUILD_TESTS OFF"
            "BUILD_SANDBOX OFF"
            "BUILD_DOC OFF"
        CMAKE_ARGS
            "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
    )

    # Create an alias target for consistency
    if(TARGET cereal AND NOT TARGET cereal::cereal)
        add_library(cereal::cereal ALIAS cereal)
    endif()
    
    message(STATUS "Successfully built cereal from source")
else()
    message(STATUS "Found cereal: ${cereal_VERSION}")
endif()

# Ensure we have the target available
if(NOT TARGET cereal::cereal AND NOT TARGET cereal)
    message(FATAL_ERROR "Could not find or build cereal library")
endif()

# Try to detect the correct cereal target
set(CEREAL_TARGET "")
foreach(target_name IN ITEMS "cereal::cereal" "cereal")
    if(TARGET ${target_name})
        set(CEREAL_TARGET ${target_name})
        break()
    endif()
endforeach()

if(NOT CEREAL_TARGET)
    message(WARNING "cereal target not found - continuing anyway to see available targets")
else()
    message(STATUS "Using cereal target: ${CEREAL_TARGET}")
endif()
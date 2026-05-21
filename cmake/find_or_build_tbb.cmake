# Find or build TBB using CPM with proper target handling
# Sets up: pagmo_tbb_wrapper target

message(STATUS "Configuring TBB dependency...")

# Force a clean TBB configuration to avoid alias conflicts
CPMFindPackage(
  NAME TBB
  GITHUB_REPOSITORY oneapi-src/oneTBB
  VERSION 2022.3.0
  GIT_TAG v2022.3.0
  OPTIONS "TBB_TEST OFF" "TBB_BUILD_TESTS OFF"
)

# Create a unified wrapper that works with any TBB target configuration
if(NOT TARGET pagmo_tbb_wrapper)
    add_library(pagmo_tbb_wrapper INTERFACE)
    
    # Check for different possible TBB target names and link appropriately
    if(TARGET TBB::tbb)
        message(STATUS "Using TBB::tbb target")
        target_link_libraries(pagmo_tbb_wrapper INTERFACE TBB::tbb)
    elseif(TARGET tbb)
        message(STATUS "Using tbb target")  
        target_link_libraries(pagmo_tbb_wrapper INTERFACE tbb)
    elseif(TARGET tbb_shared)
        message(STATUS "Using tbb_shared target")
        target_link_libraries(pagmo_tbb_wrapper INTERFACE tbb_shared)
    elseif(TARGET tbb_static)
        message(STATUS "Using tbb_static target")
        target_link_libraries(pagmo_tbb_wrapper INTERFACE tbb_static)
    else()
        message(FATAL_ERROR "No TBB target found")
    endif()
    
    # Add proper include directory if available
    if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/_deps/tbb-src/include")
        target_include_directories(pagmo_tbb_wrapper INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/_deps/tbb-src/include")
    endif()
endif()

# Debug: Check what TBB targets exist
get_property(ALL_TARGETS DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
foreach(target ${ALL_TARGETS})
    if("${target}" MATCHES "[Tt][Bb][Bb]")
        message(STATUS "Found TBB-related target: ${target}")
    endif()
endforeach()
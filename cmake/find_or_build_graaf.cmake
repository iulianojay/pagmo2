# Find or build Graaf using CPM.
# Graaf is a header-only C++20 graph library designed as a lightweight
# alternative to the Boost Graph Library (BGL).
message(STATUS "Configuring Graaf dependency...")

CPMFindPackage(
  NAME graaflib
  GITHUB_REPOSITORY bobluppes/graaf
  GIT_TAG v1.1.1
  OPTIONS
    "SKIP_TESTS ON"
    "SKIP_EXAMPLES ON"
    "SKIP_BENCHMARKS ON"
)

# Graaf does not define its own CMake targets, so we create one manually.
if(NOT TARGET Graaf::Graaf)
    add_library(Graaf::Graaf INTERFACE IMPORTED GLOBAL)
    target_include_directories(Graaf::Graaf INTERFACE "${graaflib_SOURCE_DIR}/include")
endif()

if(TARGET Graaf::Graaf)
    message(STATUS "Graaf dependency successfully configured (Graaf::Graaf).")
else()
    message(FATAL_ERROR "Failed to configure Graaf dependency.")
endif()

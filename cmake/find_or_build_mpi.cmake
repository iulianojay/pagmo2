# Find MPI using CMake's built-in FindMPI module
# Sets up: MPI::MPI_CXX target (or creates wrapper if needed)
#
# Note: Unlike TBB or Boost, MPI cannot be automatically built via CPM
# because it requires system-level installation and configuration.
# This module finds an existing MPI installation or provides
# helpful instructions for installing one.

message(STATUS "Configuring MPI dependency...")

# Try to find MPI using CMake's built-in module
find_package(MPI COMPONENTS CXX)

if(MPI_CXX_FOUND)
    message(STATUS "MPI found successfully")
    message(STATUS "  MPI C++ compiler: ${MPI_CXX_COMPILER}")
    message(STATUS "  MPI C++ compile flags: ${MPI_CXX_COMPILE_FLAGS}")
    message(STATUS "  MPI C++ include dirs: ${MPI_CXX_INCLUDE_DIRS}")
    message(STATUS "  MPI C++ libraries: ${MPI_CXX_LIBRARIES}")
    message(STATUS "  MPI version: ${MPI_CXX_VERSION}")

    # Ensure MPI::MPI_CXX target exists (should be created by FindMPI)
    if(TARGET MPI::MPI_CXX)
        message(STATUS "Using MPI::MPI_CXX target")
    else()
        # Create target if FindMPI didn't (older CMake versions)
        message(STATUS "Creating MPI::MPI_CXX target wrapper")
        add_library(MPI::MPI_CXX INTERFACE IMPORTED)
        set_target_properties(MPI::MPI_CXX PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${MPI_CXX_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${MPI_CXX_LIBRARIES}"
            INTERFACE_COMPILE_OPTIONS "${MPI_CXX_COMPILE_FLAGS}"
        )
    endif()

    # Create a pagmo-specific wrapper for consistency with other dependencies
    if(NOT TARGET pagmo_mpi_wrapper)
        add_library(pagmo_mpi_wrapper INTERFACE)
        target_link_libraries(pagmo_mpi_wrapper INTERFACE MPI::MPI_CXX)
        message(STATUS "Created pagmo_mpi_wrapper target")
    endif()

else()
    # MPI not found - provide helpful installation instructions
    message(SEND_ERROR "MPI not found. MPI is required when PAGMO_WITH_MPI=ON")
    message("")
    message("MPI must be installed at the system level. Please install an MPI implementation:")
    message("")
    message("Ubuntu/Debian:")
    message("  sudo apt-get update")
    message("  sudo apt-get install libopenmpi-dev openmpi-bin")
    message("  # OR")
    message("  sudo apt-get install libmpich-dev mpich")
    message("")
    message("RHEL/CentOS/Fedora:")
    message("  sudo yum install openmpi openmpi-devel")
    message("  # OR")
    message("  sudo dnf install mpich mpich-devel")
    message("")
    message("macOS (Homebrew):")
    message("  brew install open-mpi")
    message("  # OR")
    message("  brew install mpich")
    message("")
    message("HPC Systems:")
    message("  module load mpi")
    message("  # OR")
    message("  module load openmpi")
    message("  # OR")
    message("  module load intel-mpi")
    message("")
    message("From Source (OpenMPI - latest stable):")
    message("  # Download from https://www.open-mpi.org/software/ompi/")
    message("  wget https://download.open-mpi.org/release/open-mpi/v5.0/openmpi-5.0.6.tar.gz")
    message("  tar xzf openmpi-5.0.6.tar.gz")
    message("  cd openmpi-5.0.6")
    message("  ./configure --prefix=/usr/local")
    message("  make -j$(nproc)")
    message("  sudo make install")
    message("")
    message("From Source (MPICH - latest stable):")
    message("  # Download from https://www.mpich.org/downloads/")
    message("  wget https://www.mpich.org/static/downloads/4.2.3/mpich-4.2.3.tar.gz")
    message("  tar xzf mpich-4.2.3.tar.gz")
    message("  cd mpich-4.2.3")
    message("  ./configure --prefix=/usr/local")
    message("  make -j$(nproc)")
    message("  sudo make install")
    message("")
    message("After installing MPI, you may need to:")
    message("  - Add MPI binaries to PATH: export PATH=/usr/local/bin:$PATH")
    message("  - Add MPI libraries to LD_LIBRARY_PATH: export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH")
    message("  - Clear CMake cache and reconfigure: rm -rf build && mkdir build && cd build")
    message("  - Specify MPI location: cmake .. -DMPI_HOME=/path/to/mpi")
    message("")
    message("For more information about MPI:")
    message("  OpenMPI: https://www.open-mpi.org/")
    message("  MPICH: https://www.mpich.org/")
    message("  Intel MPI: https://software.intel.com/content/www/us/en/develop/tools/oneapi/components/mpi-library.html")
    message("")

    message(FATAL_ERROR "MPI is required but not found. Please install MPI and try again.")
endif()

# Debug: Show MPI-related targets
get_property(ALL_TARGETS DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
message(STATUS "MPI-related targets in project:")
foreach(target ${ALL_TARGETS})
    if("${target}" MATCHES "[Mm][Pp][Ii]")
        message(STATUS "  - ${target}")
    endif()
endforeach()

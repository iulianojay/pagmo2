# Find or build IPOPT using ExternalProject when enabled
# Configures: Ipopt::Ipopt target

if(PAGMO_WITH_IPOPT)
    message(STATUS "Configuring IPOPT dependency...")
    
    include(ExternalProject)

    # Map CMake build type to autoconf CFLAGS/CXXFLAGS.
    # Multi-config generators don't pick a type until build time, so fall back
    # to Release flags in that case.
    if(CMAKE_CONFIGURATION_TYPES)
        # Multi-config (MSVC, Xcode) – ipopt is autoconf anyway so this path
        # is only reached on single-config generators; default to Release.
        set(_IPOPT_BUILD_TYPE "Release")
    else()
        set(_IPOPT_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
        if(NOT _IPOPT_BUILD_TYPE)
            set(_IPOPT_BUILD_TYPE "Release")
        endif()
    endif()

    if(_IPOPT_BUILD_TYPE STREQUAL "Debug")
        set(_IPOPT_CFLAGS   "-g -O0")
        set(_IPOPT_CXXFLAGS "-g -O0")
        set(_IPOPT_FFLAGS   "-g -O0")
    elseif(_IPOPT_BUILD_TYPE STREQUAL "RelWithDebInfo")
        set(_IPOPT_CFLAGS   "-O2 -g -DNDEBUG")
        set(_IPOPT_CXXFLAGS "-O2 -g -DNDEBUG")
        set(_IPOPT_FFLAGS   "-O2 -g")
    elseif(_IPOPT_BUILD_TYPE STREQUAL "MinSizeRel")
        set(_IPOPT_CFLAGS   "-Os -DNDEBUG")
        set(_IPOPT_CXXFLAGS "-Os -DNDEBUG")
        set(_IPOPT_FFLAGS   "-Os")
    else()
        # Release (default)
        set(_IPOPT_CFLAGS   "-O3 -DNDEBUG")
        set(_IPOPT_CXXFLAGS "-O3 -DNDEBUG")
        set(_IPOPT_FFLAGS   "-O3")
    endif()
    message(STATUS "IPOPT build type: ${_IPOPT_BUILD_TYPE} (CXXFLAGS=${_IPOPT_CXXFLAGS})")

    # Use a per-build-type install directory so Debug and Release don't
    # clobber each other when CPM_SOURCE_CACHE is a shared location.
    if(DEFINED CPM_SOURCE_CACHE)
        set(_IPOPT_BASE_DIR "${CPM_SOURCE_CACHE}/ipopt/${_IPOPT_BUILD_TYPE}")
    else()
        set(_IPOPT_BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/ipopt/${_IPOPT_BUILD_TYPE}")
    endif()

    # Set the install directory
    set(IPOPT_INSTALL_DIR "${_IPOPT_BASE_DIR}_install")
    
    # Create the include directory structure beforehand
    file(MAKE_DIRECTORY ${IPOPT_INSTALL_DIR}/include/coin-or)
    file(MAKE_DIRECTORY ${IPOPT_INSTALL_DIR}/lib)
    
    # Build IPOPT from source using ExternalProject with minimal dependencies
    ExternalProject_Add(
        ipopt_external
        URL https://github.com/coin-or/Ipopt/archive/releases/3.14.12.tar.gz
        PREFIX ${_IPOPT_BASE_DIR}
        CONFIGURE_COMMAND <SOURCE_DIR>/configure 
            --prefix=${IPOPT_INSTALL_DIR}
            --disable-shared
            --enable-static
            --with-pic
            --with-lapack-lflags=-llapack\ -lblas
            --without-hsl
            --without-mumps
            --without-asl  
            --disable-linear-solver-loader
            CXX=${CMAKE_CXX_COMPILER}
            CC=${CMAKE_C_COMPILER}
            F77=gfortran
            FC=gfortran
            CFLAGS=${_IPOPT_CFLAGS}
            CXXFLAGS=${_IPOPT_CXXFLAGS}
            FFLAGS=${_IPOPT_FFLAGS}
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON
        DOWNLOAD_EXTRACT_TIMESTAMP ON
    )
    
    # Create an interface target for IPOPT (using namespaced name to avoid conflicts)
    add_library(Ipopt::Ipopt INTERFACE IMPORTED)
    add_dependencies(Ipopt::Ipopt ipopt_external)
    
    # Set include directories and libraries
    target_include_directories(Ipopt::Ipopt INTERFACE ${IPOPT_INSTALL_DIR}/include/coin-or)
    target_link_libraries(Ipopt::Ipopt INTERFACE 
        ${IPOPT_INSTALL_DIR}/lib/libipopt.a
        lapack
        blas
        gfortran
    )
    
    message(STATUS "IPOPT will be built from source with linear solver disabled")
endif()
# Find or build IPOPT using ExternalProject when enabled
# Configures: Ipopt::Ipopt target
#
# Search order:
#   1. pkg-config (ipopt.pc) — covers most system installs and conda envs
#   2. CMake config file (IpoptConfig.cmake / ipopt-config.cmake)
#   3. Manual header+library search under common prefixes
#   4. Build from source as a fallback (ExternalProject)
#
# Set IPOPT_ROOT (or the env var of the same name) to hint the search.

if(PAGMO_WITH_IPOPT)
    message(STATUS "Configuring IPOPT dependency...")

    set(_ipopt_found FALSE)

    # ------------------------------------------------------------------
    # 1. Try pkg-config
    # ------------------------------------------------------------------
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        if(DEFINED IPOPT_ROOT)
            set(ENV{PKG_CONFIG_PATH} "${IPOPT_ROOT}/lib/pkgconfig:${IPOPT_ROOT}/lib64/pkgconfig:$ENV{PKG_CONFIG_PATH}")
        endif()
        pkg_check_modules(_IPOPT_PC QUIET ipopt)
        if(_IPOPT_PC_FOUND)
            message(STATUS "Found IPOPT via pkg-config: ${_IPOPT_PC_PREFIX}")
            if(NOT TARGET Ipopt::Ipopt)
                add_library(Ipopt::Ipopt INTERFACE IMPORTED)
            endif()
            # pkg_check_modules gives lists; convert to target properties
            target_include_directories(Ipopt::Ipopt INTERFACE ${_IPOPT_PC_INCLUDE_DIRS})
            target_link_libraries(Ipopt::Ipopt INTERFACE ${_IPOPT_PC_LINK_LIBRARIES})
            target_compile_options(Ipopt::Ipopt INTERFACE ${_IPOPT_PC_CFLAGS_OTHER})
            set(_ipopt_found TRUE)
        endif()
    endif()

    # ------------------------------------------------------------------
    # 2. Try CMake config file
    # ------------------------------------------------------------------
    if(NOT _ipopt_found)
        find_package(Ipopt QUIET
            HINTS
                ${IPOPT_ROOT}
                $ENV{IPOPT_ROOT}
                /usr /usr/local /opt/coinor /opt/ipopt
            PATH_SUFFIXES lib/cmake/ipopt lib64/cmake/ipopt share/ipopt/cmake
        )
        if(Ipopt_FOUND AND TARGET Ipopt::Ipopt)
            message(STATUS "Found IPOPT via CMake config: ${Ipopt_DIR}")
            set(_ipopt_found TRUE)
        endif()
    endif()

    # ------------------------------------------------------------------
    # 3. Manual header + library search
    # ------------------------------------------------------------------
    if(NOT _ipopt_found)
        set(_ipopt_search_prefixes
            ${IPOPT_ROOT}
            $ENV{IPOPT_ROOT}
            /usr
            /usr/local
            /opt/coinor
            /opt/ipopt
        )
        find_path(_IPOPT_INCLUDE_DIR
            NAMES IpIpoptApplication.hpp
            HINTS ${_ipopt_search_prefixes}
            PATH_SUFFIXES include/coin include/coin-or include
        )
        find_library(_IPOPT_LIBRARY
            NAMES ipopt
            HINTS ${_ipopt_search_prefixes}
            PATH_SUFFIXES lib lib64
        )
        if(_IPOPT_INCLUDE_DIR AND _IPOPT_LIBRARY)
            message(STATUS "Found IPOPT (manual search): ${_IPOPT_INCLUDE_DIR}, ${_IPOPT_LIBRARY}")
            if(NOT TARGET Ipopt::Ipopt)
                add_library(Ipopt::Ipopt INTERFACE IMPORTED)
            endif()
            target_include_directories(Ipopt::Ipopt INTERFACE ${_IPOPT_INCLUDE_DIR})
            target_link_libraries(Ipopt::Ipopt INTERFACE ${_IPOPT_LIBRARY})
            set(_ipopt_found TRUE)
        endif()
        unset(_ipopt_search_prefixes)
        unset(_IPOPT_INCLUDE_DIR CACHE)
        unset(_IPOPT_LIBRARY CACHE)
    endif()

    # ------------------------------------------------------------------
    # 4. Build from source (ExternalProject fallback)
    # ------------------------------------------------------------------
    if(NOT _ipopt_found)
        message(STATUS "IPOPT not found on the system – building from source (3.14.12)")

        include(ExternalProject)

        # Map CMake build type to autoconf CFLAGS/CXXFLAGS.
        # Multi-config generators don't pick a type until build time, so fall
        # back to Release flags in that case.
        if(CMAKE_CONFIGURATION_TYPES)
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

        set(IPOPT_INSTALL_DIR "${_IPOPT_BASE_DIR}_install")

        # Create the install directory structure beforehand so that the
        # INTERFACE IMPORTED target's include path exists at configure time.
        file(MAKE_DIRECTORY ${IPOPT_INSTALL_DIR}/include/coin-or)
        file(MAKE_DIRECTORY ${IPOPT_INSTALL_DIR}/lib)

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

        add_library(Ipopt::Ipopt INTERFACE IMPORTED)
        add_dependencies(Ipopt::Ipopt ipopt_external)
        target_include_directories(Ipopt::Ipopt INTERFACE ${IPOPT_INSTALL_DIR}/include/coin-or)
        target_link_libraries(Ipopt::Ipopt INTERFACE
            ${IPOPT_INSTALL_DIR}/lib/libipopt.a
            lapack
            blas
            gfortran
        )

        message(STATUS "IPOPT will be built from source with minimal linear solver support")
    endif()

    unset(_ipopt_found)
endif()
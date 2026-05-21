# Find or build NLopt using CPM when enabled
# Configures: nlopt target

if(PAGMO_WITH_NLOPT)
    message(STATUS "Configuring NLopt dependency...")
    
    CPMFindPackage(
      NAME NLopt
      GITHUB_REPOSITORY stevengj/nlopt
      VERSION 2.10.0
      GIT_TAG v2.10.0
      OPTIONS "NLOPT_PYTHON OFF" "NLOPT_OCTAVE OFF" "NLOPT_MATLAB OFF" "NLOPT_GUILE OFF" "NLOPT_SWIG OFF" "NLOPT_TESTS OFF"
    )
    
    message(STATUS "NLopt configured successfully")
endif()
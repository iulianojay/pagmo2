# Find or build Eigen3 using CPM when enabled
# Configures: Eigen3::Eigen target

if(PAGMO_WITH_EIGEN3)
    message(STATUS "Configuring Eigen3 dependency...")
    
    CPMFindPackage(
      NAME Eigen3
      GITLAB_REPOSITORY libeigen/eigen
      VERSION 5.0.1
      GIT_TAG 5.0.1
      OPTIONS "EIGEN_BUILD_DOC OFF" "EIGEN_BUILD_TESTING OFF"
    )
    
    message(STATUS "Eigen3 configured successfully")
endif()
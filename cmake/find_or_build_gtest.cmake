# Find or build Google Test using CPM
# Configures: GTest::gtest_main, GTest::gtest

if(PAGMO_BUILD_TESTS)
    message(STATUS "Configuring Google Test dependency...")
    
    # Use CPM to fetch Google Test
    CPMFindPackage(
        NAME googletest
        GITHUB_REPOSITORY google/googletest
        GIT_TAG v1.14.0
        VERSION 1.14.0
        OPTIONS
            "INSTALL_GTEST OFF"
            "gtest_force_shared_crt ON"
    )
    
    # Check if Google Test targets were created
    if(TARGET gtest_main)
        message(STATUS "gtest_main target available")
    else()
        message(WARNING "gtest_main target not found")
    endif()
    
    if(TARGET gtest)
        message(STATUS "gtest target available")  
    else()
        message(WARNING "gtest target not found")
    endif()
    
    message(STATUS "Google Test configured successfully")
endif()
#!/bin/bash

# Script to convert Boost Test files to Google Test
# Usage: ./convert_boost_to_gtest.sh

cd /home/jay/projects/pagmo2/tests

echo "Converting Boost Test files to Google Test..."

# List of files to convert
files=$(find . -name "*.cpp" -exec grep -l "BOOST_TEST_MODULE" {} \;)

for file in $files; do
    echo "Converting $file..."
    
    # Create backup
    cp "$file" "$file.backup"
    
    # Extract module name from BOOST_TEST_MODULE
    module_name=$(grep "#define BOOST_TEST_MODULE" "$file" | sed 's/#define BOOST_TEST_MODULE \(.*\)/\1/')
    
    # Convert test framework headers and module definition
    sed -i "s/#define BOOST_TEST_MODULE.*//g" "$file"
    sed -i "s/#include <boost\/test\/unit_test.hpp>/#include <gtest\/gtest.h>/g" "$file"
    
    # Convert test cases
    sed -i "s/BOOST_AUTO_TEST_CASE(\([^)]*\))/TEST($module_name, \1)/g" "$file"
    
    # Convert assertions
    sed -i "s/BOOST_CHECK_THROW(\([^,]*\),\s*\([^)]*\))/EXPECT_THROW(\1, \2)/g" "$file"
    sed -i "s/BOOST_CHECK_NO_THROW(\([^)]*\))/EXPECT_NO_THROW(\1)/g" "$file"
    sed -i "s/BOOST_CHECK_EQUAL(\([^,]*\),\s*\([^)]*\))/EXPECT_EQ(\1, \2)/g" "$file"
    sed -i "s/BOOST_CHECK_CLOSE(\([^,]*\),\s*\([^,]*\),\s*\([^)]*\))/EXPECT_NEAR(\1, \2, \3)/g" "$file"
    sed -i "s/BOOST_CHECK(\([^)]*\))/EXPECT_TRUE(\1)/g" "$file"
    sed -i "s/BOOST_REQUIRE(\([^)]*\))/ASSERT_TRUE(\1)/g" "$file"
    sed -i "s/BOOST_REQUIRE_EQUAL(\([^,]*\),\s*\([^)]*\))/ASSERT_EQ(\1, \2)/g" "$file"
    
    echo "Converted $file"
done

echo "Conversion complete. Check the files and run tests to verify."
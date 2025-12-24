cmake_minimum_required(VERSION 3.10)
project(NanoCacheSim VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(include)

# Main Executable
add_executable(ncsim src/main.cpp)

# Testing
enable_testing()
add_executable(unit_tests test/unit/test_main.cpp)
add_test(NAME UnitTests COMMAND unit_tests)

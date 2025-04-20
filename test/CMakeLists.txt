cmake_minimum_required(VERSION 3.10)
project(intEx8_test C)

# Set the C standard
set(CMAKE_C_STANDARD 11)

# Include the intEx8 headers
include_directories(../include)

# Define the executable (test binary)
add_executable(test_intEx8 test.c ../src/intex8.c ../src/i8.c ../src/ix8.c ../src/toom3_multiply.c)

# Link math library if needed (for Toom-Cook multiplication or other math functions)
target_link_libraries(test_intEx8 m)

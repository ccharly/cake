## CMake setup
cmake_minimum_required(VERSION 3.0)

## CXX setup
set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++14")
include_directories(include)

## Examples
add_executable(examples.args examples/args.cpp)

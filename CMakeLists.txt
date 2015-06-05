cmake_minimum_required(VERSION 3.2)
project(simplevm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Add in projects now.
add_subdirectory("core")

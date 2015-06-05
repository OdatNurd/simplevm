# Set up the minimum verssion of cmake required and the overall name of the project.
cmake_minimum_required(VERSION 3.2)
project(simplevm)

# Set some flags for C++ compilation.
#
# CLion put this in here, so I'm leaving it alone, but I'm pretty sure that
# I don't care because I'm not using C++.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Add in projects now. These are essentially include commands.
add_subdirectory("core")
add_subdirectory("vm")

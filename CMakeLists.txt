cmake_minimum_required(VERSION 3.2)
project(simplevm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    src/context.c
    src/context.h
    src/main.c
    src/opcodes.h
    src/registers.h
    src/vm.c
    src/vm.h)

add_executable(vm ${SOURCE_FILES})

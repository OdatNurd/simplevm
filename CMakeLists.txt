cmake_minimum_required(VERSION 3.2)
project(simplevm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    vm/context.c
    vm/context.h
    vm/main.c
    vm/opcodes.h
    vm/registers.h
    vm/vm.c
    vm/vm.h)

add_executable(vm ${SOURCE_FILES})

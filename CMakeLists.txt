cmake_minimum_required(VERSION 3.2)
project(simplevm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    context.c
    context.h
    main.c
    opcodes.h
    registers.h
    vm.c
    vm.h)

add_executable(simplevm ${SOURCE_FILES})
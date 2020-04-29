cmake_minimum_required(VERSION 3.15)
project(Main)

set(CMAKE_CXX_STANDARD 14)

include_directories(Qunit)
include_directories(Qunit/headers)
include_directories(Stack)
include_directories(Stack/headers)

add_executable(Main
        Qunit/headers/QUnit.h
        Stack/headers/Stack.h
        StackTest/source/StackQUnit.cc)

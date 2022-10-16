cmake_minimum_required(VERSION 3.21)

project(diploma)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Ofast")

add_executable(naive src/main.cpp)
cmake_minimum_required(VERSION 3.21)

project(diploma)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3")

set (SOURCES
        src/main.cpp
        src/hadamard_matrix/hadamard_matrix.cpp
        src/matrix_printer/matrix_printer.cpp
        src/bucket/bucket.cpp)

add_executable(naive ${SOURCES})

target_include_directories (naive
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include/hadamard_matrix
            ${CMAKE_SOURCE_DIR}/include/matrix_printer
            ${CMAKE_SOURCE_DIR}/include/bucket)
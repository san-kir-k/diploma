cmake_minimum_required(VERSION 3.25)

project(diploma)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O2 -mcpu=apple-m1")

set (SOURCES
        src/main.cpp
        src/hadamard_matrix/hadamard_matrix.cpp
        src/matrix_printer/matrix_printer.cpp
        src/bucket/bucket.cpp
        src/row/row.cpp
        src/utils/mm_finder.cpp
        src/utils/cli_args_parser.cpp
        src/utils/q_classifier.cpp)

add_executable(naive ${SOURCES})

target_include_directories (naive
        PRIVATE
        ${CMAKE_SOURCE_DIR}/include/hadamard_matrix
        ${CMAKE_SOURCE_DIR}/include/matrix_printer
        ${CMAKE_SOURCE_DIR}/include/bucket
        ${CMAKE_SOURCE_DIR}/include/row
        ${CMAKE_SOURCE_DIR}/include/matrix
        ${CMAKE_SOURCE_DIR}/include/utils)
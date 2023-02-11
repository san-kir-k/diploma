#pragma once

#include <cstdint>
#include <iostream>
#include "matrix.h"
#include "row.h"

inline void DEBUG_PRINT_ROW(const Row& row, uint64_t order)
{
    for (auto i = 0; i < order; ++i)
    {
        std::cout << ((row.Data() & (1 << (order - i - 1))) != 0);
    }
}

inline void DEBUG_PRINT_MATRIX(const Matrix& matrix)
{
    auto order = matrix.Order();

    for (auto i = 0; i < order; ++i)
    {
        DEBUG_PRINT_ROW(matrix[i], order);
        std::cout << "\n";
    }
    std::cout << "\n\n";
}
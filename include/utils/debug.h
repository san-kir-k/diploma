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

inline void DEBUG_PRINT_ROW_PM(const Row& row, uint64_t order)
{
    for (auto i = 0; i < order; ++i)
    {
        std::cout << (((row.Data() & (1 << (order - i - 1))) != 0) == 1 ? '+' : '-');
    }
}

inline void DEBUG_PRINT_MATRIX(const Matrix& matrix)
{
    auto order = matrix.Order();

    std::cout << "\n";
    for (auto i = 0; i < order; ++i)
    {
        DEBUG_PRINT_ROW_PM(matrix[i], order);
        std::cout << "\n";
    }
    std::cout << "\n";
}

inline void DEBUG_PRINT_MATRIX_MASK(const Matrix& lhs, const Matrix& rhs)
{
    auto order = lhs.Order();

    std::cout << "\n";
    for (auto i = 0; i < order; ++i)
    {
        DEBUG_PRINT_ROW_PM(lhs[i], order);
        std::cout << "    ";
        DEBUG_PRINT_ROW_PM(lhs[i] ^ rhs[i], order);
        std::cout << "    ";
        DEBUG_PRINT_ROW_PM(rhs[i], order);
        std::cout << "\n";
    }
    std::cout << "\n";
}
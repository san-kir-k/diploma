#pragma once

#include <vector>
#include <unordered_set>

#include "matrix.h"
#include "row.h"

inline constexpr uint64_t Rho(const Row& row)
{
    return row.Data();
}

// так как у меня матрица с убывающими столбцами и строками, а не возрастающими как в статье, то алгоритм немного поменялся
void Core(Matrix& result, Matrix& h, uint64_t r, bool flag);

Matrix GetMinimalMatrix(const Matrix& m, const std::unordered_set<std::string>& memo = {});
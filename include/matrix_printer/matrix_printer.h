#pragma once
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <bitset>
#include <iomanip>
#include <filesystem>
#include <string>

#include "matrix.h"

namespace fs = std::filesystem;

// класс для печати матриц в файлики
class MatrixPrinter
{
public:
    explicit MatrixPrinter(uint64_t order, const std::string& genDirname);

    void PrintMatrix(const Matrix& m, uint64_t eqClass) const;
    void PrintMatrices(const std::vector<Matrix>& matrices) const;

private:
    uint64_t    m_order;
    std::string m_genDirname;
    std::string m_mDirname;
};
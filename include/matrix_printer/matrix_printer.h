#pragma once
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <bitset>
#include <iomanip>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// класс для печати матриц в файлики
class MatrixPrinter
{
public:
    explicit MatrixPrinter(uint64_t order);

    void PrintMatrix(const std::vector<std::bitset<64>>& m, uint64_t eqClass) const;

private:
    uint64_t    m_order;
    std::string m_genDirname;
    std::string m_mDirname;
};
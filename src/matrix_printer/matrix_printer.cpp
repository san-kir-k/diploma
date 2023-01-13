#include "matrix_printer.h"

MatrixPrinter::MatrixPrinter(uint64_t order)
        : m_order(order)
        , m_genDirname("../generated_matrices/")
{
    m_mDirname = m_genDirname + "order_" + std::to_string(m_order);
    fs::create_directory(m_mDirname);
}

void MatrixPrinter::PrintMatrix(const std::vector<std::bitset<64>>& m, uint64_t eqClass) const
{
    std::ofstream commonOut(m_mDirname + "/eq_" + std::to_string(eqClass) + "_common.txt");
    std::ofstream pmOut(m_mDirname + "/eq_" + std::to_string(eqClass) + "_pm_view.txt");
    auto order = m.size();
    for (const auto& row: m)
    {
        for (auto i = 0; i < order; ++i)
        {
            commonOut << std::setw(3) << (row[order - i - 1] == 0 ? "-1 " : "1 ");
            pmOut << (row[order - i - 1] == 0 ? "- " : "+ ");
        }
        commonOut << "\n";
        pmOut << "\n";
    }
}
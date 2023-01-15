#include "hadamard_matrix.h"

#include <filesystem>
#include <fstream>
#include <chrono>

#include "matrix.h"
#include "mm_finder.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

HadamardMatrix::HadamardMatrix(uint64_t order, Mode mode)
    : m_order(order)
    , m_bucket(order, mode)
    , m_mode(mode)
    , m_printer(order, "../build/generated_matrices/")
{
    m_bucket.GenerateMatrix();
}

void HadamardMatrix::GetResult() const
{
    if (m_mode == Mode::NORMAL || m_mode == Mode::UNIQUE_ONLY)
    {
        const auto& res = m_bucket.GetFoundMatrices();
        std::cout << "[RESULT] : Count of matrices with order = "
                  << m_order << " : "
                  << res.size() << "\n";

        for (auto i = 0; i < res.size(); ++i)
        {
            m_printer.PrintMatrix(res[i], i + 1);
        }
    }
    else
    {
        auto res = m_bucket.GetCountOfFoundMatrices();
        std::cout << "[RESULT] : Count of matrices with order = "
                  << m_order << " : "
                  << res << "\n";
    }
}

void HadamardMatrix::FindMinimalMatrix(const std::string& filename, uint64_t num)
{
    std::ifstream in(filename);

    std::string strOrder;
    std::getline(in, strOrder);
    uint64_t order = std::stoi(strOrder);

    Matrix m{order};

    std::string line;
    auto rowCounter = 0;
    while (std::getline(in, line))
    {
        for (auto colCounter = 0; colCounter < line.size(); ++colCounter)
        {
            auto c = line[colCounter];
            if (c == '+')
            {
                m[rowCounter][order - colCounter - 1] = 1;
            }
            else
            {
                m[rowCounter][order - colCounter - 1] = 0;
            }
        }
        ++rowCounter;
    }

    MatrixPrinter printer{order, "../build/minimal_matrices/"};

    auto tStart = high_resolution_clock::now();
    auto minM = GetMinimalMatrix(m);
    auto tEnd = high_resolution_clock::now();

    auto ms = duration_cast<milliseconds>(tEnd - tStart);

    std::cout << "[TIME] : order = " << order
              << " : time = " << ms.count() / 6000 << " : "
                              << ms.count() / 100 << " : "
                              << ms.count() % 100 << " [min:sec:ms]\n";

    printer.PrintMatrix(minM, num);
}
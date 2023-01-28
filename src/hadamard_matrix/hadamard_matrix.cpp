#include "hadamard_matrix.h"

#include <filesystem>
#include <fstream>
#include <chrono>

#include "matrix.h"
#include "mm_finder.h"
#include "q_classifier.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::seconds;

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

Matrix HadamardMatrix::GetMatrixFromFile(const std::string& filename)
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

    return m;
}

void HadamardMatrix::FindMinimalMatrix(const std::string& filename, uint64_t num)
{
    Matrix m = GetMatrixFromFile(filename);
    auto order = m.Order();

    MatrixPrinter printer{order, "../build/minimal_matrices/"};

    auto tStart = high_resolution_clock::now();
    auto minM = GetMinimalMatrix(m);
    auto tEnd = high_resolution_clock::now();

    auto sec = duration_cast<seconds>(tEnd - tStart);

    std::cout << "[TIME] : order = " << order
              << " : time = " << sec.count() / 60 << ":"
              << sec.count() % 60 << " [min:sec]\n";

    printer.PrintMatrix(minM, num);
}

void HadamardMatrix::FindQClasses(const std::string& dirname)
{
    using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

    std::vector<Matrix> inMatrices;
    for (const auto& filename : recursive_directory_iterator(dirname))
    {
        auto matrix = GetMatrixFromFile(filename.path().string());
        inMatrices.push_back(matrix);
    }

    Classifier classifier{inMatrices.front().Order()};

    auto tStart = high_resolution_clock::now();
    std::vector<uint64_t> qClasses = classifier.Classify(inMatrices);
    auto tEnd = high_resolution_clock::now();

    auto sec = duration_cast<seconds>(tEnd - tStart);

    std::cout << "[TIME] : time = "
              << sec.count() / 60 << ":"
              << sec.count() % 60 << " [min:sec]\n";

    std::cout << "Found " << qClasses.size() << " distinct Q Classes:\n";
    for (auto i = 0; i < qClasses.size(); ++i)
    {
        std::cout << " > Matrix number " << i + 1 << " has " << qClasses[i] + 1 << " Q class\n";
    }
}
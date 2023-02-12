#include "hadamard_matrix.h"

#include <filesystem>
#include <fstream>
#include <chrono>
#include <unordered_set>
#include <utility>
#include <string>

#include "matrix.h"
#include "mm_finder.h"
#include "q_classifier.h"
#include "debug.h"

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

bool HadamardMatrix::GetMatrixFromFileCore(std::ifstream& in, uint64_t order, Matrix& m)
{
    bool result = false;
    std::string line;
    auto rowCounter = 0;
    while (rowCounter < order && std::getline(in, line))
    {
        result = true;
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
    return result;
}

Matrix HadamardMatrix::GetMatrixFromFile(const std::string& filename)
{
    std::ifstream in(filename);

    std::string strOrder;
    std::getline(in, strOrder);
    uint64_t order = std::stoi(strOrder);

    Matrix result{order};
    GetMatrixFromFileCore(in, order, result);

    return result;
}

std::vector<Matrix> HadamardMatrix::GetMatricesFromFile(const std::string& filename)
{
    std::vector<Matrix> result;

    std::ifstream in(filename);
    std::string strOrder;
    std::getline(in, strOrder);
    uint64_t order = std::stoi(strOrder);

    Matrix m{order};
    while (GetMatrixFromFileCore(in, order, m))
    {
        result.push_back(m);
    }

    return result;
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

void HadamardMatrix::FindMinimalMatrices(const std::string& filename)
{
    std::vector<Matrix> minMatrices;
    std::vector<Matrix> matrices = GetMatricesFromFile(filename);
    auto order = matrices.front().Order();

    MatrixPrinter printer{order, "../build/minimal_matrices/"};

    auto tStart = high_resolution_clock::now();
    for (const auto& m: matrices)
    {
        minMatrices.push_back(GetMinimalMatrix(m));
    }
    auto tEnd = high_resolution_clock::now();

    auto sec = duration_cast<seconds>(tEnd - tStart);

    std::cout << "[TIME] : order = " << order
              << " : time = " << sec.count() / 60 << ":"
              << sec.count() % 60 << " [min:sec]\n";

    printer.PrintMatrices(minMatrices);
}

void HadamardMatrix::FindQClasses(const std::string& dirname, bool singleFile)
{
    using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

    std::vector<Matrix> inMatrices;
    if (!singleFile)
    {
        for (const auto& filename : recursive_directory_iterator(dirname))
        {
            auto matrix = GetMatrixFromFile(filename.path().string());
            inMatrices.push_back(std::move(matrix));
        }
    }
    else
    {
        for (const auto& filename : recursive_directory_iterator(dirname))
        {
            inMatrices = std::move(GetMatricesFromFile(filename.path().string()));
        }
    }

    Classifier classifier;
    if (singleFile)
    {
        std::unordered_set<std::string> minMatricesSet;
        for (const auto& m: inMatrices)
        {
            minMatricesSet.insert(m.ToString());
        }
        classifier = Classifier{inMatrices.front().Order(), std::move(minMatricesSet)};
    }
    else
    {
        classifier = Classifier{inMatrices.front().Order()};
    }

    auto tStart = high_resolution_clock::now();
    std::vector<uint64_t> qClasses = classifier.Classify(inMatrices);
    auto tEnd = high_resolution_clock::now();

    auto sec = duration_cast<seconds>(tEnd - tStart);

    std::cout << "\n================================================================\n"
              << "[TIME] : time = "
              << sec.count() / 60 << ":"
              << sec.count() % 60 << " [min:sec]\n";

    std::cout << "Found "
              << std::unordered_set<uint64_t>(qClasses.begin(), qClasses.end()).size()
              << " distinct Q Classes:\n";
    for (auto i = 0; i < qClasses.size(); ++i)
    {
        std::cout << " > Matrix number " << i + 1 << " has " << qClasses[i] + 1 << " Q class\n";
        DEBUG_PRINT_MATRIX(inMatrices[i]);
    }
}
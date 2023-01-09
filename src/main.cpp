//
// Created by AK on 05.10.2022.
//

/**
 *
 * 1111...1111
 * 1111...0000
 * 1100...1100
 * 7 6    5 4
 *
 * Берем строчку (111…111), ставим на первое месте, относительно нее сделать множество,
 * это множество назовем корзиной (оно ортогонально певрой строке),
 * из этой корзины выбираем строку самую первую и размещаем на второй позиции,
 * потом строим множество, в котором все строки ортогональны первым двум строкам.
 *
 * Предыдущую корзину и выбранную строку и просеиваем корзину, оставляя только те,
 * которые ортогональны выбранной. Если корзина опустела до конца,
 * то возвращаемся на шаг назад, то текущую строку менять на следующую строку из корзины и продолжить алгоритм.
 */

#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <vector>
#include <cassert>
#include <bitset>
#include <iomanip>
#include <filesystem>
#include <limits>
#include <deque>

namespace fs = std::filesystem;

class MatrixPrinter
{
public:
    explicit MatrixPrinter(uint64_t order)
        : m_order(order)
        , m_genDirname("../generated_matrices/")
    {
        m_mDirname = m_genDirname + "order_" + std::to_string(m_order);
        fs::create_directory(m_mDirname);
    }

    void PrintMatrix(const std::vector<std::bitset<64>>& m, uint64_t eqClass)
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

private:
    uint64_t    m_order;
    std::string m_genDirname;
    std::string m_mDirname;
};


class Bucket
{
public:
    using Row           = std::bitset<64>;
    using Matrix        = std::vector<Row>;
    using HistoryBucket = std::pair<std::deque<Row>, uint64_t>;

public:
    explicit Bucket(uint64_t order, bool countOnly)
        : m_order(order)
        , m_completedRows()
        , m_completedCols(order)
        , m_bucketHistory()
        , m_countOfFoundMatrices(0)
        , m_foundMatrices()
        , m_countOnly(countOnly)
        , m_printer(order)
    {
        assert(m_order <= 64 && m_order >= 1 && (m_order % 2 == 0 || m_order == 1));
        // init basis
        uint64_t row = 0;
        for (auto i = 0; i < m_order; ++i)
        {
            row |= (1 << i);
        }
        uint64_t maxVal = row;
        m_completedRows.push_back(row);
        row = 0;
        for (auto i = 0; i < m_order / 2; ++i)
        {
            row |= (1 << (m_order - i - 1));
        }
        if (row != 0)
        {
            m_completedRows.push_back(row);
        }
        row = 0;
        for (auto i = 0; i < m_order / 4; ++i)
        {
            row |= (1 << (m_order - i - 1));
            row |= (1 << (m_order / 2 - i - 1));
        }
        if (row != 0)
        {
            m_completedRows.push_back(row);
        }
        // reduce init bucket
        auto reduced = ReduceInitBucket(maxVal);
        m_bucketHistory.push_back({reduced, 0});
        for (auto i = 0; i < m_order; ++i)
        {
            m_completedCols[i] = 7 - (4 * i / m_order);
        }
    }

    void GenerateMatrix()
    {
        // matrices of order <= 2
        if (m_completedRows.size() == m_order)
        {
//            std::cout << "[INFO] : Matrix of order " << m_order << " found\n";
            m_foundMatrices.push_back(m_completedRows);
            m_countOfFoundMatrices = 1;
            return;
        }

        while (m_bucketHistory.size() != 0)
        {
            Row   nextRow;
            auto& [bucket, chosen] = m_bucketHistory.back();
            for (; chosen < bucket.size(); ++chosen)
            {
                nextRow = bucket[chosen];
                if (IsDecreasing(m_completedCols, nextRow, m_order))
                {
                    ++chosen;
                    break;
                }
            }
            if (nextRow.count() == 0)
            {
                m_completedRows.pop_back();
                for (auto& col: m_completedCols)
                {
                    col >>= 1;
                }
                m_bucketHistory.pop_back();
                continue;
            }
            auto reduced = ReduceBucket(nextRow, bucket);
            m_completedRows.push_back(nextRow);
            for (auto i = 0; i < m_order; ++i)
            {
                m_completedCols[i] <<= 1;
                m_completedCols[i] |= std::bitset<64>(nextRow[m_order - i - 1]);
            }
            m_bucketHistory.push_back({reduced, 0});
            if (m_completedRows.size() == m_order)
            {
//                std::cout << "[INFO] : Matrix of order " << m_order << " found\n";
                if (m_countOnly)
                {
                    ++m_countOfFoundMatrices;
                }
                else
                {
                    m_foundMatrices.push_back(m_completedRows);
                }
                m_completedRows.pop_back();
                for (auto& col: m_completedCols)
                {
                    col >>= 1;
                }
                m_bucketHistory.pop_back(); 
            }
        }
    }

    void CountFoundMatrices()
    {
        std::cout << "[RESULT] : Count of matrices with order = "
                  << m_order << " : "
                  << (m_countOnly ? m_countOfFoundMatrices : m_foundMatrices.size()) << "\n";
    }

    void PrintFoundMatrices()
    {
        for (auto i = 0; i < m_foundMatrices.size(); ++i)
        {
            m_printer.PrintMatrix(m_foundMatrices[i], i + 1);
        }
    }

    ~Bucket() = default;

private:
    std::deque<Row> ReduceBucket(const Row& row, const std::deque<Row>& vec)
    {
        std::deque<Row> reduced;
        for (const auto& r: vec)
        {
            if (IsOrthogonal(row, r, m_order) && row.to_ullong() > r.to_ullong())
            {
                reduced.push_back(r);
            }
        }
        return reduced;
    }

    std::deque<Row> ReduceInitBucket(uint64_t maxVal)
    {
        std::deque<Row> reduced;
        for (auto num = maxVal - 1; num != maxVal / 2; --num)
        {
            bool isOrtho = true;
            for (const auto& row: m_completedRows)
            {
                if (!IsOrthogonal(row, num, m_order))
                {
                    isOrtho = false;
                }
            }
            if (isOrtho)
            {
                reduced.push_back(num);
            }
        }
        return reduced;
    }

    static bool IsOrthogonal(const Row& lhs, const Row& rhs, uint64_t order)
    {
        return (lhs ^ rhs).count() == (order / 2);
    }

    static bool IsDecreasing(const Matrix& upper, const Row& lower, uint64_t order)
    {
        std::vector<uint64_t> seq;
        for (auto i = 0; i < order; ++i)
        {
            const auto& col = upper[i];
            seq.push_back(((col << 1) | std::bitset<64>(lower[order - i - 1])).to_ullong());
        }
        for (int i = 0; i < order - 1; ++i)
        {
            if (seq[i] < seq[i + 1])
            {
                return false;
            }
        }
        return true;
    }

private:
    uint64_t                  m_order;
    Matrix                    m_completedRows;
    Matrix                    m_completedCols;
    std::deque<HistoryBucket> m_bucketHistory;
    uint64_t                  m_countOfFoundMatrices;
    std::vector<Matrix>       m_foundMatrices;
    bool                      m_countOnly;
    MatrixPrinter             m_printer;
};

class HadamardMatrixBuilder
{
public:
    explicit HadamardMatrixBuilder(uint64_t order, bool countOnly = false)
        : m_bucket(order, countOnly)
    {
        m_bucket.GenerateMatrix();
    }

    void CountMatrices()
    {
        m_bucket.CountFoundMatrices();
    }

    void PrintMatrices()
    {
        m_bucket.PrintFoundMatrices();
    }

    ~HadamardMatrixBuilder() = default;

private:
    Bucket m_bucket;
};

int main(int argc, char** argv)
{
    size_t n = 1;
    while (n <= 20)
    {
        if (argc > 1 && std::strcmp(argv[1], "COUNT_ONLY") == 0)
        {
            HadamardMatrixBuilder b(n, true);
            b.CountMatrices();
        }
        else
        {
            HadamardMatrixBuilder b(n);
            b.PrintMatrices();
        }
        if (n < 4)
        {
            n *= 2;
        }
        else
        {
            n += 4;
        }
    }
    return 0;
}
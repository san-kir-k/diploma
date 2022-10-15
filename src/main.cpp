//
// Created by AK on 05.10.2022.
//

/**
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
    explicit Bucket(uint64_t order)
        : m_order(order)
        , m_completedRows()
        , m_bucketHistory()
        , m_foundMatrices()
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
    }

    void GenerateMatrix()
    {
        // matrices of order <= 2
        if (m_completedRows.size() == m_order)
        {
            std::cout << "[INFO] : Matrices of order " << m_order << " found\n";
            m_foundMatrices.push_back(m_completedRows);
            return;
        }

        while (m_bucketHistory.size() != 0)
        {
            Row   nextRow;
            auto& [bucket, chosen] = m_bucketHistory.back();
            for (; chosen < bucket.size(); ++chosen)
            {
                nextRow = bucket[chosen];
                if (IsDecreasing(m_completedRows, nextRow, m_order))
                {
                    ++chosen;
                    break;
                }
            }
            if (nextRow.count() == 0)
            {
                m_completedRows.pop_back();
                m_bucketHistory.pop_back();
                continue;
            }
            auto reduced = ReduceBucket(nextRow, bucket);
            m_completedRows.push_back(nextRow);
            m_bucketHistory.push_back({reduced, 0});
            if (m_completedRows.size() == m_order)
            {
                std::cout << "[INFO] : Matrices of order " << m_order << " found\n";
                m_foundMatrices.push_back(m_completedRows);
                m_completedRows.pop_back();
                m_bucketHistory.pop_back();
                break;
            }
        }
    }

    void CountFoundMatrices()
    {
        std::cout << "[RESULT] : Count of matrices with order = "
                  << m_order << " : " << m_foundMatrices.size() << "\n";
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
            if (IsOrthogonal(row, r, m_order))
            {
                reduced.push_back(r);
            }
        }
        return reduced;
    }

    std::deque<Row> ReduceInitBucket(uint64_t maxVal)
    {
        std::deque<Row> reduced;
        for (auto num = maxVal - 1; num != 0; --num)
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
        for (int64_t i = 0; i < order - 1; ++i)
        {
            uint64_t lhs = 0;
            uint64_t rhs = 0;
            for (auto j = 0; j < upper.size(); ++j)
            {
                lhs |= (upper[j][i] << (upper.size() - j));
                rhs |= (upper[j][i + 1] << (upper.size() - j));
            }
            lhs |= lower[i];
            rhs |= lower[i + 1];
            if (lhs > rhs)
            {
                return false;
            }
        }
        return true;
    }

private:
    uint64_t                  m_order;
    Matrix                    m_completedRows;
    std::deque<HistoryBucket> m_bucketHistory;
    std::vector<Matrix>       m_foundMatrices;
    MatrixPrinter             m_printer;
};

class HadamardMatrixBuilder
{
public:
    explicit HadamardMatrixBuilder(uint64_t order) : m_bucket(order)
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
        HadamardMatrixBuilder b(n);
        if (argc > 1 && std::strcmp(argv[1], "COUNT_ONLY") == 0)
        {
            b.CountMatrices();
        }
        else
        {
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
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
#include <vector>
#include <cassert>
#include <bitset>
#include <iomanip>
#include <filesystem>
#include <stack>
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
    using Row    = std::bitset<64>;
    using Matrix = std::vector<Row>;

public:
    explicit Bucket(uint64_t order)
        : m_order(order)
        , m_completedRows()
        , m_foundMatrices()
        , m_printer(order)
        , m_bucketHistory()
        , m_maxVal(0)
    {
        assert(m_order <= 64 && m_order >= 1 && (m_order % 2 == 0 || m_order == 1));
        m_bucketHistory.push(std::deque<Row>(0));
        for (auto i = 0; i < m_order; ++i)
        {
            m_maxVal |= (1 << i);
        }
    }

    void GenerateMatrix()
    {
        std::deque<Row> reduced(m_order);
        m_completedRows.push_back(m_maxVal);
        auto nextFirst = m_maxVal - 1;
        while (nextFirst != m_maxVal)
        {
            if (m_bucketHistory.top().size() == 0)
            {
                reduced = ReduceBucket(m_completedRows.back());
            }
            else
            {
                reduced = ReduceBucket(m_completedRows.back(),
                                       m_bucketHistory.top());
            }
            if (m_completedRows.size() == m_order)
            {
                std::cout << "[INFO] : Matrices of order " << m_order << " found\n";
                m_foundMatrices.push_back(m_completedRows);
                // TODO: Надо понять, как искать матрицы всех классов эквивалентности
                break;
            }
            if (reduced.size() > 0)
            {
                m_bucketHistory.push(reduced);
                m_completedRows.push_back(reduced.front());
            }
            else
            {
                if (m_bucketHistory.top().size() > 1)
                {
                    m_completedRows.pop_back();
                }
                else
                {
                    m_completedRows.pop_back();
                    m_completedRows.pop_back();
                    m_bucketHistory.pop();
                }
                if (m_bucketHistory.top().size() > 0)
                {
                    m_bucketHistory.top().pop_front();
                    m_completedRows.push_back(m_bucketHistory.top().front());
                }
                else
                {
                    m_completedRows.push_back(nextFirst);
                    --nextFirst;
                }
            }
        }
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

    std::deque<Row> ReduceBucket(const Row& row)
    {
        std::deque<Row> reduced;
        auto num = m_maxVal - 1;
        while (true)
        {
            if (IsOrthogonal(row, num, m_order))
            {
                reduced.push_back(num);
            }
            if (num == 0)
            {
                break;
            }
            --num;
        }
        return reduced;
    }

    static bool IsOrthogonal(const Row& lhs, const Row& rhs, uint64_t order)
    {
        return (lhs ^ rhs).count() == (order / 2);
    }

private:
    uint64_t                    m_order;
    Matrix                      m_completedRows;
    std::vector<Matrix>         m_foundMatrices;
    MatrixPrinter               m_printer;
    std::stack<std::deque<Row>> m_bucketHistory;
    uint64_t                    m_maxVal;
};

class HadamardMatrixBuilder
{
public:
    explicit HadamardMatrixBuilder(uint64_t order) : m_bucket(order)
    {
        m_bucket.GenerateMatrix();
    }

    void PrintMatrices()
    {
        m_bucket.PrintFoundMatrices();
    }

    ~HadamardMatrixBuilder() = default;

private:
    Bucket m_bucket;
};

int main()
{
    size_t n = 1;
    while (n <= 32)
    {
        HadamardMatrixBuilder b1(n);
        b1.PrintMatrices();
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
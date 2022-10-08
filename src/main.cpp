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
    {
        assert(m_order <= 64 && m_order >= 1 && (m_order % 2 == 0 || m_order == 1));
        uint64_t first = 0;
        for (uint64_t i = 0; i < m_order; ++i)
        {
            first |= (1ULL << i);
        }
        m_completedRows.emplace_back(first);
    }

    void GenerateMatrix()
    {
        auto next = m_completedRows.back().to_ullong();
        while (m_completedRows.size() != 0)
        {
            while (next != 0)
            {
                --next;
                if (IsOrthogonal(next))
                {
                    m_completedRows.emplace_back(next);
                }
            }
            if (m_completedRows.size() == m_order)
            {
                m_foundMatrices.push_back(m_completedRows);
                break; // Надо понять, как искать матрицы всех классов эквивалентности
            }
            next = m_completedRows.back().to_ullong();
            m_completedRows.pop_back();
        }
        std::cout << "[INFO] : Matrices of order " << m_order << " found\n";
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
    bool IsOrthogonal(const Row& other)
    {
        for (const auto& row: m_completedRows)
        {
            if ((row ^ other).count() != (m_order / 2))
            {
                return false;
            }
        }
        return true;
    }

private:
    uint64_t            m_order;
    Matrix              m_completedRows;
    std::vector<Matrix> m_foundMatrices;
    MatrixPrinter       m_printer;
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
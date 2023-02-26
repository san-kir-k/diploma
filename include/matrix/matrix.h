#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <string>

#include "row.h"

class Matrix
{
public:
    explicit Matrix(uint64_t order): m_order(order), m_matrix(order, Row{order}) {}
    explicit Matrix(uint64_t order, uint64_t size): m_order(order), m_matrix(size) {}
    explicit Matrix(uint64_t order, const Row& row): m_order(order), m_matrix(order, row) {}

    Matrix(const Matrix&) = default;
    Matrix& operator=(const Matrix&) = default;

    Matrix(Matrix&&) = default;
    Matrix& operator=(Matrix&&) = default;

    ~Matrix() = default;

    inline Row operator[](uint64_t pos) const
    {
        return m_matrix[pos];
    }
    inline Row& operator[](uint64_t pos)
    {
        return m_matrix[pos];
    }

    inline uint64_t ColumnData(uint64_t colPos) const
    {
        uint64_t result = 0;
        auto shift = 0;
        for (const auto& row: m_matrix)
        {
            result |= (row[colPos] << shift);
            ++shift;
        }
        return result;
    }

    inline std::string ToString() const
    {
        std::string result;
        for (const auto& row: m_matrix)
        {
            result += "$" + std::to_string(row.Data());
        }
        return result;
    }

    inline uint64_t Size() const
    {
        return m_matrix.size();
    }

    inline constexpr uint64_t Order() const
    {
        return m_order;
    }

    inline constexpr const std::vector<Row>& Data() const
    {
        return m_matrix;
    }
    inline constexpr std::vector<Row>& Data()
    {
        return m_matrix;
    }

    inline void PopBack()
    {
        m_matrix.pop_back();
    }
    inline void PushBack(const Row& row)
    {
        m_matrix.push_back(row);
    }
    inline void PushBack(Row&& row)
    {
        m_matrix.push_back(std::move(row));
    }

    // в статье было сказано, что нормализовать надо только операциями из CR, CC
    inline constexpr void Normalize()
    {
        // normalize rows
        for (auto r = 0; r < m_order; ++r)
        {
            if (!m_matrix[r][m_order - 1])
            {
                for (auto c = 0; c < m_order; ++c)
                {
                    m_matrix[r][c] = !m_matrix[r][c];
                }
            }
        }

        // normalize columns
        for (auto c = 0; c < m_order; ++c)
        {
            if (!m_matrix[0][c])
            {
                for (auto r = 0; r < m_order; ++r)
                {
                    m_matrix[r][c] = !m_matrix[r][c];
                }
            }
        }
    }

    inline constexpr void ColumnsSwap(uint64_t i, uint64_t j)
    {
        for (auto r = 0; r < m_order; ++r)
        {
            swap(m_matrix[r][m_order - i - 1], m_matrix[r][m_order - j - 1]);
        }
    }

    inline void RowsSwap(uint64_t i, uint64_t j)
    {
        std::swap(m_matrix[i], m_matrix[j]);
    }

    inline Matrix GetTransposed()
    {
        Matrix transposed{m_order};

        for (auto i = 0; i < m_order; ++i)
        {
            for (auto j = 0; j < m_order; ++j)
            {
                transposed[i][m_order - j - 1] = m_matrix[j][m_order - i - 1];
            }
        }
        
        return transposed;
    }

    inline Matrix RowsAreCols()
    {
        Matrix transposed{m_order};

        for (auto i = 0; i < m_order; ++i)
        {
            for (auto j = 0; j < m_order; ++j)
            {
                transposed[i][j] = m_matrix[j][i];
            }
        }

        return transposed;
    }

    inline void ColumnSort()
    {
        Matrix transposed{m_order};
        for (auto i = 0; i < m_order; ++i)
        {
            for (auto j = 0; j < m_order; ++j)
            {
                transposed[i].Store(m_order - j - 1, m_matrix[j].Get(m_order - i - 1));
            }
        }

        std::sort(transposed.m_matrix.begin(), transposed.m_matrix.end(), [](const Row& lhs, const Row& rhs) {
            return lhs > rhs;
        });

        for (auto i = 0; i < m_order; ++i)
        {
            for (auto j = 0; j < m_order; ++j)
            {
                m_matrix[i].Store(m_order - j - 1, transposed[j].Get(m_order - i - 1));
            }
        }
    }

    inline bool IsHadamard() const
    {
        for (auto start = 0; start < m_order; ++start)
        {
            for (auto next = start + 1; next < m_order; ++next)
            {
                auto rowXOR = m_matrix[start] ^ m_matrix[next];
                if (rowXOR.Count() != m_order / 2)
                {
                    return false;
                }
            }
        }
        return true;
    }

private:
    uint64_t         m_order;
    std::vector<Row> m_matrix;
};
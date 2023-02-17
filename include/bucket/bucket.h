#pragma once
#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>
#include <deque>
#include <unordered_set>
#include <string>

#include "matrix.h"
#include "row.h"
#include "mode.h"
#include "mm_finder.h"

/**
 *         Здесь просто написан алгоритм примерный
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

class Bucket
{
public:
    explicit Bucket(uint64_t order, Mode mode);

    void GenerateMatrix();

    const std::vector<Matrix>& GetFoundMatrices() const;
    uint64_t GetCountOfFoundMatrices() const;

    ~Bucket() = default;

private:
    using BucketContext = std::pair<std::deque<Row>, uint64_t>;

private:
    // просеить корзину, оставив только те строки, что ортогональны строке row и меньше ее
    std::deque<Row> ReduceBucket(const Row& row, const std::deque<Row>& vec);

    // просеить начальную корзину по базису из трех строк
    std::deque<Row> ReduceInitBucket(uint64_t maxVal);

    // проверить, что при дабавлении к незавершенной матрице upper новой строки lower
    // столбцы полученной матрицы будут в строго убывающем порядке
    static bool IsDecreasing(const Matrix& upper, const Row& lower);

private:
    uint64_t                        m_order;

    Matrix                          m_completedRows;
    Matrix                          m_completedCols;

    std::deque<BucketContext>       m_bucketHistory;

    uint64_t                        m_countOfFoundMatrices;
    std::vector<Matrix>             m_foundMatrices;
    std::vector<Matrix>             m_foundUniqueMatrices;
    std::unordered_set<std::string> m_UniqueMatricesSet;

    Mode                            m_mode;
};
#pragma once
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <vector>
#include <cassert>
#include <bitset>
#include <iomanip>
#include <limits>
#include <deque>
#include <algorithm>
#include <unordered_set>
#include <string>

#include "matrix_printer.h"

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
    // строка матрицы
    using Row           = std::bitset<64>;
    // сама матрица
    using Matrix        = std::vector<Row>;
    // тип для истории корзин с контекстом -- pair<тип корзины; строка, на которой остановилось просеивание>
    using HistoryBucket = std::pair<std::deque<Row>, uint64_t>;

public:
    explicit Bucket(uint64_t order, bool countOnly);

    void GenerateMatrix();

    void CountFoundMatrices() const;

    // вывод найденных матриц
    void PrintFoundMatrices() const;

    ~Bucket() = default;

private:
    // просеить корзину, оставив только те строки, что ортогональны строке row и меньше ее
    std::deque<Row> ReduceBucket(const Row& row, const std::deque<Row>& vec);

    // просеить начальную корзину по базису из трех строк
    std::deque<Row> ReduceInitBucket(uint64_t maxVal);

    static bool IsOrthogonal(const Row& lhs, const Row& rhs, uint64_t order);

    // проверить, что при дабавлении к незавершенной матрице upper новой строки lower
    // столбцы полученной матрицы будут в строго убывающем порядке
    static bool IsDecreasing(const Matrix& upper, const Row& lower, uint64_t order);

    // в статье было сказано, что нормализовать надо только операциями из CR, CC
    static void NormalizeMatrix(Matrix& m, uint64_t order);

    static void ColumnsSwap(Matrix& m, uint64_t i, uint64_t j);

    static void RowsSwap(Matrix& m, uint64_t i, uint64_t j);

    static void ColumnSort(Matrix& m);

    static uint64_t Rho(const Row& row);

    // так как у меня матрица с убывающими столбцами и строками, а не возрастающими как в статье, то алгоритм немного поменялся
    static void Core(Matrix& result, Matrix& h, uint64_t order, uint64_t r, bool flag);

    static Matrix GetMinimumMatrix(const Matrix& m, uint64_t order);

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
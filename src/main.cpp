//
// Created by AK on 05.10.2022.
//

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
#include <algorithm>
#include <unordered_set>
#include <string>

namespace fs = std::filesystem;

// класс для печати матриц в файлики
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
    // строка матрицы
    using Row           = std::bitset<64>;
    // сама матрица
    using Matrix        = std::vector<Row>;
    // тип для истории корзин с контекстом -- pair<тип корзины; строка, на которой остановилось просеивание>
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
        // first three rows of matrix
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
        // столбцы нужны, чтобы потом проверять и поддерживать свойство убывания столбцов и колонок в матрице
        for (auto i = 0; i < m_order; ++i)
        {
            m_completedCols[i] = 7 - (4 * i / m_order);
        }
    }

    void GenerateMatrix()
    {
        // set for minimal matrices (пока в качестве локальной переменной с очень говорящим названием)
        // чтобы set работал, я храню матрицу в качестве строки (линеаризую матрицу),
        // то есть [111..11, 11..00, ..] -> "11..1111..00..." но в десятичном представлении чисел
        std::unordered_set<std::string> s;
        // case for matrices of order <= 2
        if (m_completedRows.size() == m_order)
        {
            m_foundMatrices.push_back(m_completedRows);
            m_countOfFoundMatrices = 1;
            return;
        }

        // bucket algo
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
            // если нашли матрицу адамара, то сохраняем ее минимальное представление в set
            if (m_completedRows.size() == m_order)
            {
                // если не нужно выводить в файл матрицы, а только подсчитать их число
                if (m_countOnly)
                {
                    auto eq_m = GetMinimumMatrix(m_completedRows, m_order);
                    std::string sm = "";
                    for (const auto& row: eq_m)
                    {
                        sm += std::to_string(row.to_ullong());
                    }
                    s.insert(sm);
                    ++m_countOfFoundMatrices;
                }
                else
                {
                    auto eq_m = GetMinimumMatrix(m_completedRows, m_order);
                    std::string sm = "";
                    for (const auto& row: eq_m)
                    {
                        sm += std::to_string(row.to_ullong());
                    }
                    s.insert(sm);
                    std::cout << s.size() << "\n";
                    // созранить матрицу в первоначальном виде
                    // (пока осталось как наследие от прошлой версии, где просто искались матрицы без эквивалентности
                    // может, оно уже и не надо)
                    m_foundMatrices.push_back(m_completedRows);
                }
                // бэктрекинг
                m_completedRows.pop_back();
                for (auto& col: m_completedCols)
                {
                    col >>= 1;
                }
                m_bucketHistory.pop_back(); 
            }
        }
        // пока только для дебажного вывода
        std::cout << s.size() << "\n";
    }

    // вывод числа найденных матриц
    void CountFoundMatrices()
    {
        std::cout << "[RESULT] : Count of matrices with order = "
                  << m_order << " : "
                  << (m_countOnly ? m_countOfFoundMatrices : m_foundMatrices.size()) << "\n";
    }

    // вывод найденных матриц
    void PrintFoundMatrices()
    {
        for (auto i = 0; i < m_foundMatrices.size(); ++i)
        {
            m_printer.PrintMatrix(m_foundMatrices[i], i + 1);
        }
    }

    ~Bucket() = default;

private:
    // просеить корзину, оставив только те строки, что ортогональны строке row и меньше ее
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

    // просеить начальную корзину по базису из трех строк
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

    // проверить, что при дабавлении к незавершенной матрице upper новой строки lower
    // столбцы полученной матрицы будут в строго убывающем порядке
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

    // в статье было сказано, что нормализовать надо только операциями из CR, CC
    static void NormalizeMatrix(Matrix& m, uint64_t order)
    {
        // normalize rows
        for (auto r = 0; r < order; ++r)
        {
            if (!m[r][order - 1])
            {
                for (auto c = 0; c < order; ++c)
                {
                    m[r][c] = ~m[r][c];
                }
            }
        }

        // normalize columns
        for (auto c = 0; c < order; ++c)
        {
            if (!m[0][c])
            {
                for (auto r = 0; r < order; ++r)
                {
                    m[r][c] = ~m[r][c];
                }
            }
        }
    }

    static void ColumnsSwap(Matrix& m, uint64_t i, uint64_t j)
    {
        auto order = m.size();
        for (auto r = 0; r < order; ++r)
        {
            std::swap(m[r][order - i - 1], m[r][order - j - 1]);
        }
    }

    static void RowsSwap(Matrix& m, uint64_t i, uint64_t j)
    {
        std::swap(m[i], m[j]);
    }

    static void ColumnSort(Matrix& m)
    {
        auto order = m.size();
        Matrix transposed(order);
        for (auto i = 0; i < order; ++i)
        {
            for (auto j = 0; j < order; ++j)
            {
                transposed[i][order - j - 1] = m[j][order - i - 1];
            }
        }
        std::sort(transposed.begin(), transposed.end(), [](const Row& lhs, const Row& rhs) {
            return lhs.to_ullong() > rhs.to_ullong();
        });
        for (auto i = 0; i < order; ++i)
        {
            for (auto j = 0; j < order; ++j)
            {
                m[i][order - j - 1] = transposed[j][order - i - 1];
            }
        }
    }

    static uint64_t Rho(const Row& row)
    {
        return row.to_ullong();
    }

    // так как у меня матрица с убывающими столбцами и строками, а не возрастающими как в статье, то алгоритм немного поменялся
    static void Core(Matrix& result, Matrix& h, uint64_t order, uint64_t r, bool flag)
    {
        if (r == order - 1)
        {
            ColumnSort(h);
            if (flag || Rho(h[r]) > Rho(result[r]))
            {
                result[r] = h[r];
            }
            return;
        }

        Row m;
        auto k = -1;
        std::vector<uint64_t> row_candidates(order, 0);

        for (auto i = r; i < order; ++i)
        {
            RowsSwap(h, i, r);
            ColumnSort(h);
            if (Rho(h[r]) == Rho(m))
            {
                ++k;
                row_candidates[k] = i;
            }
            if (Rho(h[r]) > Rho(m))
            {
                k = 0;
                row_candidates[k] = i;
                m = h[r];
            }
            RowsSwap(h, i, r);
        }

        if (flag || Rho(m) > Rho(result[r]))
        {
            result[r] = m;
            RowsSwap(h, r, row_candidates[0]);
            ColumnSort(h);
            Core(result, h, order, r + 1, true);
            RowsSwap(h, r, row_candidates[0]);

            for (auto i = 1; i <= k; ++i)
            {
                RowsSwap(h, r, row_candidates[i]);
                ColumnSort(h);
                Core(result, h, order, r + 1, false);
                RowsSwap(h, r, row_candidates[i]);
            }
        }
        if (!flag && m == result[r])
        {
            for (auto i = 0; i <= k; ++i)
            {
                RowsSwap(h, r, row_candidates[i]);
                ColumnSort(h);
                Core(result, h, order, r + 1, false);
                RowsSwap(h, r, row_candidates[i]);
            }
        }
    }

    static Matrix GetMinimumMatrix(const Matrix& m, uint64_t order)
    {
        auto result = m;
        NormalizeMatrix(result, order);
        auto h = m;

        for (auto j = 0; j < order; ++j)
        {
            ColumnsSwap(h, 0, j);
            for (auto i = 0; i < order; ++i)
            {
                RowsSwap(h, 0, i);
                NormalizeMatrix(h, order);
                Core(result, h, order, 1, false);
                RowsSwap(h, 0, i);
            }
            h = m;
        }

        return result;
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

// не очень полезный по итогу класс
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
    while (n <= 16)
    {
        // если запускать без COUNT_ONLY, то программа будет выводить матрицы в файлики
        // а если с COUNT_ONLY, то только подсчитает их число
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
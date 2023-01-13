#include "bucket.h"

Bucket::Bucket(uint64_t order, bool countOnly)
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

void Bucket::GenerateMatrix()
{
    // set for minimal matrices (пока в качестве локальной переменной с очень говорящим названием)
    // чтобы set работал, я храню матрицу в качестве строки (линеаризую матрицу),
    // то есть [111..11, 11..00, ..] -> "11..1111..00..." но в десятичном представлении чисел
    std::unordered_set<std::string> s;
    std::vector<Bucket::Matrix> foundMinimalMatrices;
    // case for matrices of order <= 2
    if (m_completedRows.size() == m_order)
    {
        m_foundMatrices.push_back(m_completedRows);
        m_countOfFoundMatrices = 1;

        // debug only
        for (auto i = 0; i < m_foundMatrices.size(); ++i)
        {
            m_printer.PrintMatrix(m_foundMatrices[i], i + 1);
        }
        return;
    }

    // bucket algo
    while (m_bucketHistory.size() != 0)
    {
        Bucket::Row   nextRow;
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
                if (!s.count(sm))
                {
                    foundMinimalMatrices.push_back(eq_m);
                    s.insert(sm);
                }
                // сохранить матрицу в первоначальном виде
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
    for (auto i = 0; i < foundMinimalMatrices.size(); ++i)
    {
        m_printer.PrintMatrix(foundMinimalMatrices[i], i + 1);
    }
}

// вывод числа найденных матриц
void Bucket::CountFoundMatrices() const
{
    std::cout << "[RESULT] : Count of matrices with order = "
              << m_order << " : "
              << (m_countOnly ? m_countOfFoundMatrices : m_foundMatrices.size()) << "\n";
}

// вывод найденных матриц
void Bucket::PrintFoundMatrices() const
{
    for (auto i = 0; i < m_foundMatrices.size(); ++i)
    {
        m_printer.PrintMatrix(m_foundMatrices[i], i + 1);
    }
}

// просеить корзину, оставив только те строки, что ортогональны строке row и меньше ее
std::deque<Bucket::Row> Bucket::ReduceBucket(const Bucket::Row& row, const std::deque<Bucket::Row>& vec)
{
    std::deque<Bucket::Row> reduced;
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
std::deque<Bucket::Row> Bucket::ReduceInitBucket(uint64_t maxVal)
{
    std::deque<Bucket::Row> reduced;
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

bool Bucket::IsOrthogonal(const Bucket::Row& lhs, const Bucket::Row& rhs, uint64_t order)
{
    return (lhs ^ rhs).count() == (order / 2);
}

// проверить, что при дабавлении к незавершенной матрице upper новой строки lower
// столбцы полученной матрицы будут в строго убывающем порядке
bool Bucket::IsDecreasing(const Bucket::Matrix& upper, const Bucket::Row& lower, uint64_t order)
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
void Bucket::NormalizeMatrix(Bucket::Matrix& m, uint64_t order)
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

void Bucket::ColumnsSwap(Bucket::Matrix& m, uint64_t i, uint64_t j)
{
    auto order = m.size();
    for (auto r = 0; r < order; ++r)
    {
        std::swap(m[r][order - i - 1], m[r][order - j - 1]);
    }
}

void Bucket::RowsSwap(Bucket::Matrix& m, uint64_t i, uint64_t j)
{
    std::swap(m[i], m[j]);
}

void Bucket::ColumnSort(Bucket::Matrix& m)
{
    auto order = m.size();
    Bucket::Matrix transposed(order);
    for (auto i = 0; i < order; ++i)
    {
        for (auto j = 0; j < order; ++j)
        {
            transposed[i][order - j - 1] = m[j][order - i - 1];
        }
    }
    std::sort(transposed.begin(), transposed.end(), [](const Bucket::Row& lhs, const Bucket::Row& rhs) {
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

uint64_t Bucket::Rho(const Bucket::Row& row)
{
    return row.to_ullong();
}

// так как у меня матрица с убывающими столбцами и строками, а не возрастающими как в статье, то алгоритм немного поменялся
void Bucket::Core(Bucket::Matrix& result, Bucket::Matrix& h, uint64_t order, uint64_t r, bool flag)
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

    Bucket::Row m;
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

Bucket::Matrix Bucket::GetMinimumMatrix(const Bucket::Matrix& m, uint64_t order)
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
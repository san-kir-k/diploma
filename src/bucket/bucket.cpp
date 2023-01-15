#include "bucket.h"

Bucket::Bucket(uint64_t order, Mode mode)
    : m_order(order)
    , m_completedRows(order, 0)
    , m_completedCols(order)
    , m_bucketHistory()
    , m_countOfFoundMatrices(0)
    , m_foundMatrices()
    , m_foundUniqueMatrices()
    , m_mode(mode)
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
    m_completedRows.PushBack(Row{m_order, row});
    row = 0;
    for (auto i = 0; i < m_order / 2; ++i)
    {
        row |= (1 << (m_order - i - 1));
    }
    if (row != 0)
    {
        m_completedRows.PushBack(Row{m_order, row});
    }
    row = 0;
    for (auto i = 0; i < m_order / 4; ++i)
    {
        row |= (1 << (m_order - i - 1));
        row |= (1 << (m_order / 2 - i - 1));
    }
    if (row != 0)
    {
        m_completedRows.PushBack(Row{m_order, row});
    }
    // reduce init bucket
    auto reduced = ReduceInitBucket(maxVal);
    m_bucketHistory.push_back({reduced, 0});
    // столбцы нужны, чтобы потом проверять и поддерживать свойство убывания столбцов и колонок в матрице
    for (auto i = 0; i < m_order; ++i)
    {
        m_completedCols[i] = Row{m_order, 0b111 - (4 * i / m_order)};
    }
}

void Bucket::GenerateMatrix()
{
    // case for matrices of order <= 2
    if (m_completedRows.Size() == m_order)
    {
        if (m_mode == Mode::NORMAL)
        {
            m_foundMatrices.push_back(m_completedRows);
        }
        else if (m_mode == Mode::COUNT_ONLY)
        {
            m_countOfFoundMatrices = 1;
        }
        else
        {
            m_foundUniqueMatrices.push_back(m_completedRows);
        }
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
            if (IsDecreasing(m_completedCols, nextRow))
            {
                ++chosen;
                break;
            }
        }
        if (nextRow.Count() == 0)
        {
            m_completedRows.PopBack();
            for (auto& col: m_completedCols.Data())
            {
                col >>= 1;
            }
            m_bucketHistory.pop_back();
            continue;
        }
        auto reduced = ReduceBucket(nextRow, bucket);
        m_completedRows.PushBack(nextRow);
        for (auto i = 0; i < m_order; ++i)
        {
            m_completedCols[i] <<= 1;
            m_completedCols[i] |= nextRow[m_order - i - 1];
        }
        m_bucketHistory.push_back(BucketContext{reduced, 0});
        // если нашли матрицу адамара, то сохраняем ее минимальное представление в set
        if (m_completedRows.Size() == m_order)
        {
            if (m_mode == Mode::NORMAL)
            {
                m_foundMatrices.push_back(m_completedRows);
            }
            else if (m_mode == Mode::COUNT_ONLY)
            {
                ++m_countOfFoundMatrices;
            }
            else
            {
                auto minMatrix = GetMinimalMatrix(m_completedRows);
                if (!m_UniqueMatricesSet.count(minMatrix.ToString()))
                {
                    m_foundUniqueMatrices.push_back(minMatrix);
                    m_UniqueMatricesSet.insert(minMatrix.ToString());
                }
            }
            // бэктрекинг
            m_completedRows.PopBack();
            for (auto& col: m_completedCols.Data())
            {
                col >>= 1;
            }
            m_bucketHistory.pop_back();
        }
    }
}

// просеить корзину, оставив только те строки, что ортогональны строке row и меньше ее
std::deque<Row> Bucket::ReduceBucket(const Row& row, const std::deque<Row>& vec)
{
    std::deque<Row> reduced;
    for (const auto& r: vec)
    {
        if (AreOrthogonal(row, r) && row > r)
        {
            reduced.push_back(r);
        }
    }
    return reduced;
}

// просеить начальную корзину по базису из трех строк
std::deque<Row> Bucket::ReduceInitBucket(uint64_t maxVal)
{
    std::deque<Row> reduced;
    for (auto num = maxVal - 1; num != maxVal / 2; --num)
    {
        Row candidate = Row{m_order, num};
        bool isOrtho = true;
        for (const auto& row: m_completedRows.Data())
        {
            if (!AreOrthogonal(row, candidate))
            {
                isOrtho = false;
            }
        }
        if (isOrtho)
        {
            reduced.push_back(candidate);
        }
    }
    return reduced;
}

// проверить, что при дабавлении к незавершенной матрице upper новой строки lower
// столбцы полученной матрицы будут в строго убывающем порядке
bool Bucket::IsDecreasing(const Matrix& upper, const Row& lower)
{
    auto order = upper.Order();
    std::vector<Row> seq;
    for (auto i = 0; i < order; ++i)
    {
        const auto& col = upper[i];
        seq.push_back((col << 1) | lower[order - i - 1]);
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

const std::vector<Matrix>& Bucket::GetFoundMatrices() const
{
    if (m_mode == Mode::NORMAL)
    {
        return m_foundMatrices;
    }
    else if (m_mode == Mode::UNIQUE_ONLY)
    {
        return m_foundUniqueMatrices;
    }
    else
    {
        assert(false);
    }

    // to suspend warnings
    return m_foundMatrices;
}

uint64_t Bucket::GetCountOfFoundMatrices() const
{
    if (m_mode == Mode::COUNT_ONLY)
    {
        return m_countOfFoundMatrices;
    }
    else if (m_mode == Mode::NORMAL)
    {
        return m_foundMatrices.size();
    }
    else
    {
        return m_foundUniqueMatrices.size();
    }
}
#include "mm_finder.h"

void Core(Matrix& result, Matrix& h, uint64_t r, bool flag)
{
    auto order = result.Order();

    if (r == order - 1)
    {
        h.ColumnSort();
        if (flag || Rho(h[r]) > Rho(result[r]))
        {
            result[r] = h[r];
        }
        return;
    }

    Row m;
    auto k = -1;
    std::vector<uint64_t> rowCandidates(order, 0);

    for (auto i = r; i < order; ++i)
    {
        h.RowsSwap(i, r);
        h.ColumnSort();
        if (Rho(h[r]) == Rho(m))
        {
            ++k;
            rowCandidates[k] = i;
        }
        if (Rho(h[r]) > Rho(m))
        {
            k = 0;
            rowCandidates[k] = i;
            m = h[r];
        }
        h.RowsSwap(i, r);
    }

    if (flag || Rho(m) > Rho(result[r]))
    {
        result[r] = m;
        h.RowsSwap(r, rowCandidates[0]);
        h.ColumnSort();
        Core(result, h, r + 1, true);
        h.RowsSwap(r, rowCandidates[0]);

        for (auto i = 1; i <= k; ++i)
        {
            h.RowsSwap(r, rowCandidates[i]);
            h.ColumnSort();
            Core(result, h, r + 1, false);
            h.RowsSwap(r, rowCandidates[i]);
        }
    }
    if (!flag && m == result[r])
    {
        for (auto i = 0; i <= k; ++i)
        {
            h.RowsSwap(r, rowCandidates[i]);
            h.ColumnSort();
            Core(result, h, r + 1, false);
            h.RowsSwap(r, rowCandidates[i]);
        }
    }
}

Matrix GetMinimalMatrix(const Matrix& m)
{
    auto order = m.Order();
    
    auto result = m;
    result.Normalize();
    auto h = m;

    for (auto j = 0; j < order; ++j)
    {
        h.ColumnsSwap(0, j);
        for (auto i = 0; i < order; ++i)
        {
            h.RowsSwap(0, i);
            h.Normalize();
            Core(result, h, 1, false);
            h.RowsSwap(0, i);
        }
        h = m;
    }

    return result;
}
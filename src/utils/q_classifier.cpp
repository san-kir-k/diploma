#include "q_classifier.h"
#include "mm_finder.h"

#include <iostream>
#include <bitset>

std::vector<uint64_t> Classifier::Classify(const std::vector<Matrix>& matrices) const
{
    if (m_order % 8 == 4)
    {
        return Classify4mod8(matrices);
    }
    else
    {
        return Classify0mod8(matrices);
    }
}

void Classifier::GetNextPos(uint64_t offset, uint64_t count,
                                std::vector<std::vector<uint64_t>>& storage,
                                std::vector<uint64_t>& tmp) const
{
    if (count == 0)
    {
        storage.push_back(tmp);
        return;
    }

    for (auto i = offset; i <= m_order - count; ++i)
    {
        tmp.push_back(i);
        GetNextPos(i + 1, count - 1, storage, tmp);
        tmp.pop_back();
    }
}

std::vector<std::vector<uint64_t>> Classifier::GetAllPos(uint64_t count) const
{
    std::vector<std::vector<uint64_t>> result;
    std::vector<uint64_t> tmp;
    GetNextPos(0, count, result, tmp);
    return result;
}

//bool Classifier::CheckQuadruple(const Matrix& matrix,
//                                const std::vector<uint64_t>& colsPos,
//                                const std::vector<uint64_t>& rowsPos) const
//{
//    uint64_t rowMask = 0;
//    for (auto next: colsPos)
//    {
//        rowMask |= (1 << next);
//    }
//
//    Row cumulative = matrix[rowsPos.front()] & rowMask;
//    for (auto next = 1; next < rowsPos.size(); ++next)
//    {
//        cumulative = cumulative ^ (matrix[rowsPos[next]] & rowMask);
//        if (!(cumulative.Count() == m_order / 4 || cumulative.Count() == 0))
//        {
//            return false;
//        }
//    }
//
//    return true;
//}

bool Classifier::CheckRowQuadruple(const Matrix& matrix,
                                   const std::vector<uint64_t>& rowsPos) const
{
    Row cumulative = matrix[rowsPos.front()];
    for (auto next = 1; next < rowsPos.size(); ++next)
    {
        cumulative = cumulative ^ matrix[rowsPos[next]];
    }

    return cumulative.Count() == m_order || cumulative.Count() == 0;
}

bool Classifier::CheckColQuadruple(const Matrix& matrix,
                                   const std::vector<uint64_t>& colsPos) const
{
    std::vector<Row> columns;

    for (auto next: colsPos)
    {
        uint64_t data = 0;
        for (auto pos = 0; pos < m_order; ++pos)
        {
            data |= (matrix[pos][next] << pos);
        }
        columns.emplace_back(m_order, data);
    }

    Row cumulative = columns.front();
    for (auto next = 1; next < columns.size(); ++next)
    {
        cumulative = cumulative ^ columns[next];
    }

    return cumulative.Count() == m_order || cumulative.Count() == 0;
}

bool Classifier::CheckRowQClass(const Matrix& matrix,
                                const std::vector<uint64_t>& colsPos,
                                const std::vector<uint64_t>& rowsPos) const
{
    uint64_t rowMask = 0;
    for (auto next: colsPos)
    {
        rowMask |= (1 << next);
    }

    Row cumulative = matrix[rowsPos.front()] & rowMask;
    auto countOfOnes = cumulative.Count();
    for (auto next = 1; next < rowsPos.size(); ++next)
    {
        if ((matrix[rowsPos[next]] & rowMask).Count() == countOfOnes
            || (matrix[rowsPos[next]] & rowMask).Count() == colsPos.size() - countOfOnes)
        {
            cumulative = cumulative ^ (matrix[rowsPos[next]] & rowMask);
            if (!(cumulative.Count() == colsPos.size() || cumulative.Count() == 0))
            {
                return false;
            }
        }
    }

    return true;
}

bool Classifier::CheckColQClass(const Matrix& matrix,
                                const std::vector<uint64_t>& colsPos,
                                const std::vector<uint64_t>& rowsPos) const
{
    std::vector<Row> columns;

    for (auto next: colsPos)
    {
        uint64_t data = 0;
        for (auto pos: rowsPos)
        {
            data |= (matrix[pos][next] << pos);
        }
        columns.emplace_back(m_order, data);
    }

    Row cumulative = columns.front();
    auto countOfOnes = cumulative.Count();
    for (auto next = 1; next < columns.size(); ++next)
    {
        if (columns[next].Count() == countOfOnes
            || columns[next].Count() == rowsPos.size() - countOfOnes)
        {
            cumulative = cumulative ^ columns[next];
            if (!(cumulative.Count() == rowsPos.size() || cumulative.Count() == 0))
            {
                return false;
            }
        }
    }

    return true;
}

Matrix Classifier::GetNewMatrix(const Matrix& matrix,
                                const std::vector<uint64_t>& colsPos,
                                const std::vector<uint64_t>& rowsPos) const
{
    Matrix newMatrix = matrix;

    uint64_t rowMask = 0;
    for (auto next: colsPos)
    {
        rowMask |= (1 << next);
    }

    for (auto next: rowsPos)
    {
        auto row = newMatrix[next];
        newMatrix[next] = (row & ~rowMask) | (~row & rowMask);
    }

    return newMatrix;
}

std::vector<uint64_t> Classifier::Classify0mod8(const std::vector<Matrix>& matrices) const
{
    std::vector<uint64_t> result;

    auto nextQClass = -1;
    auto rowsPositions = GetAllPos(4);
    auto colsPositions = GetAllPos(m_order / 4);
    int snum = 1;
    for (const auto& startMatrix: matrices)
    {
        std::cout << "[DEBUG] : [!!!] : New start matrix number " << snum << "\n";
        ++snum;
        auto minStartMatrix = GetMinimalMatrix(startMatrix);
        auto strMatrix = minStartMatrix.ToString();
        if (m_cachedQClasses.size() > 0 && m_cachedQClasses.back().count(strMatrix))
        {
            result.push_back(nextQClass);
            continue;
        }
        else
        {
            ++nextQClass;
            result.push_back(nextQClass);
            m_cachedQClasses.emplace_back();
            m_cachedQClasses.back().insert(strMatrix);
            std::cout << "[DEBUG] : [!!!] : New Q class : " << nextQClass << "\n";
        }
        std::vector<Matrix> candidates;
        candidates.push_back(minStartMatrix);
        for (auto next = 0; next < candidates.size(); ++next)
        {
            std::cout << "[DEBUG] : [!!!] : New candidate number " << next + 1 << "\n";
            auto matrix = candidates[next];

            for (const auto& nextRowPos: rowsPositions)
            {
                if (CheckRowQuadruple(matrix, nextRowPos))
                {
                    std::cout << "[DEBUG] : [QR] : Found quadruple\n";
                    for (const auto& nextColPos: colsPositions)
                    {
                        if (CheckRowQClass(matrix, nextColPos, nextRowPos))
                        {
                            // TODO: print positions for debug
                            auto nextGeneratedMatrix = GetNewMatrix(matrix, nextColPos, nextRowPos);
                            auto minNextGeneratedMatrix = GetMinimalMatrix(nextGeneratedMatrix);
                            auto strGenMatrix = minNextGeneratedMatrix.ToString();
                            if (!m_cachedQClasses[nextQClass].count(strGenMatrix))
                            {
                                std::cout << "[DEBUG] : [QR] : Constructed new matrix by switch : " << candidates.size() << "\n";
                                m_cachedQClasses[nextQClass].insert(strGenMatrix);
                                candidates.push_back(minNextGeneratedMatrix);
                            }
                            break;
                        }
                    }
                }
            }

            // transposed pattern
            for (const auto& nextColPos: rowsPositions)
            {
                if (CheckColQuadruple(matrix, nextColPos))
                {
                    std::cout << "[DEBUG] : [QC] : Found quadruple\n";
                    for (const auto& nextRowPos: colsPositions)
                    {
                        if (CheckColQClass(matrix, nextColPos, nextRowPos))
                        {
                            // TODO: print positions for debug
                            auto nextGeneratedMatrix = GetNewMatrix(matrix, nextColPos, nextRowPos);
                            auto minNextGeneratedMatrix = GetMinimalMatrix(nextGeneratedMatrix);
                            auto strGenMatrix = minNextGeneratedMatrix.ToString();
                            if (!m_cachedQClasses[nextQClass].count(strGenMatrix))
                            {
                                std::cout << "[DEBUG] : [QC] : Constructed new matrix by switch : " << candidates.size() << "\n";
                                m_cachedQClasses[nextQClass].insert(strGenMatrix);
                                candidates.push_back(minNextGeneratedMatrix);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    return result;
}

std::vector<uint64_t> Classifier::Classify4mod8(const std::vector<Matrix>& matrices) const
{
    // TODO: ...
    return {};
}
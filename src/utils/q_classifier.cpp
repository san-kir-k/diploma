#include "q_classifier.h"
#include "mm_finder.h"

#include <iostream>

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

void Classifier::GetNextRowsPos(uint64_t offset, uint64_t count,
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
        GetNextRowsPos(i + 1, count - 1, storage, tmp);
        tmp.pop_back();
    }
}

std::vector<std::vector<uint64_t>> Classifier::GetAllRowsPos(uint64_t count) const
{
    std::vector<std::vector<uint64_t>> result;
    std::vector<uint64_t> tmp;
    GetNextRowsPos(0, count, result, tmp);
    return result;
}

bool Classifier::CheckQuadruple(const Matrix& matrix,
                                const std::vector<uint64_t>& colsPos,
                                const std::vector<uint64_t>& rowsPos) const
{
    uint64_t rowMask = 0;
    for (auto next: rowsPos)
    {
        rowMask |= (1 << next);
    }

    Row cumulative = matrix[colsPos.front()];
    for (auto next = 1; next < colsPos.size(); ++next)
    {
        cumulative = cumulative ^ (matrix[colsPos[next]] & rowMask);
        if (!(cumulative.Count() == m_order / 4 && cumulative.Count() == 0))
        {
            return false;
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
    for (auto next: rowsPos)
    {
        rowMask |= (1 << next);
    }

    for (auto next: colsPos)
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
    auto colsPositions = GetAllRowsPos(3);
    auto rowsPositions = GetAllRowsPos(m_order / 4);
    for (const auto& startMatrix: matrices)
    {
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
            std::cout << "[!!!] : [DEBUG] : New Q class : " << nextQClass << "\n";
        }
        std::vector<Matrix> candidates;
        candidates.push_back(minStartMatrix);
        for (auto next = 0; next < candidates.size(); ++next)
        {
            const auto& matrix = candidates[next];

            for (const auto& nextColPos: colsPositions)
            {
                for (const auto& nextRowPos: rowsPositions)
                {
                    if (CheckQuadruple(matrix, nextColPos, nextRowPos))
                    {
                        // TODO: print positions for debug
                        auto nextGeneratedMatrix = GetNewMatrix(matrix, nextColPos, nextRowPos);
                        auto minNextGeneratedMatrix = GetMinimalMatrix(nextGeneratedMatrix);
                        auto strGenMatrix = minNextGeneratedMatrix.ToString();
                        if (!m_cachedQClasses[nextQClass].count(strGenMatrix))
                        {
                            std::cout << "[DEBUG] : Constructed new matrix by switch : " << candidates.size() << "\n";
                            m_cachedQClasses[nextQClass].insert(strGenMatrix);
                            candidates.push_back(minNextGeneratedMatrix);
                        }
                    }
                }
            }

            // transposed pattern
            for (const auto& nextColPos: rowsPositions)
            {
                for (const auto& nextRowPos: colsPositions)
                {
                    if (CheckQuadruple(matrix, nextColPos, nextRowPos))
                    {
                        // TODO: print positions for debug
                        auto nextGeneratedMatrix = GetNewMatrix(matrix, nextColPos, nextRowPos);
                        auto minNextGeneratedMatrix = GetMinimalMatrix(nextGeneratedMatrix);
                        auto strGenMatrix = minNextGeneratedMatrix.ToString();
                        if (!m_cachedQClasses[nextQClass].count(strGenMatrix))
                        {
                            std::cout << "[DEBUG] : Constructed new matrix by switch : " << candidates.size() << "\n";
                            m_cachedQClasses[nextQClass].insert(strGenMatrix);
                            candidates.push_back(minNextGeneratedMatrix);
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
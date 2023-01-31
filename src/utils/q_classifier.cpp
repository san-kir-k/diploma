#include "q_classifier.h"
#include "mm_finder.h"

#include <iostream>
#include <algorithm>

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

bool Classifier::CheckRowQuadruple(const Matrix& matrix,
                                   const std::vector<uint64_t>& rowsPos,
                                   uint64_t& lastRowPosResult) const
{
    Row cumulative = matrix[rowsPos.front()];
    for (auto next = 1; next < rowsPos.size(); ++next)
    {
        cumulative = cumulative ^ matrix[rowsPos[next]];
    }

    for (auto lastRowPos = rowsPos.back() + 1; lastRowPos < matrix.Order(); ++lastRowPos)
    {
        if (matrix[lastRowPos] == cumulative || matrix[lastRowPos] == ~cumulative)
        {
            lastRowPosResult = lastRowPos;
            return true;
        }
    }

    return false;
}

bool Classifier::CheckRowQClass(const Matrix& matrix,
                                const std::vector<uint64_t>& colsPos,
                                const std::vector<uint64_t>& rowsPos) const
{
    std::vector<Row> blocks;

    for (auto rowPos: rowsPos)
    {
        blocks.emplace_back(m_order);
        for (auto i = 0; i < colsPos.size(); ++i)
        {
            auto colPos = colsPos[i];
            blocks.back() |= (matrix[rowPos][colPos] << i);
        }
    }

    auto prev = blocks.front();
    for (auto i = 1; i < blocks.size(); ++i)
    {
        if (!(blocks[i] == prev || blocks[i] == ~prev))
        {
            return false;
        }
        prev = blocks[i];
    }

    return true;
}

Matrix Classifier::GetNewMatrix(const Matrix& matrix,
                                const std::vector<uint64_t>& colsPos,
                                const std::vector<uint64_t>& rowsPos,
                                uint64_t lastRowPos) const
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
    newMatrix[lastRowPos] = (newMatrix[lastRowPos] & ~rowMask) | (~newMatrix[lastRowPos] & rowMask);

    return newMatrix;
}

std::vector<uint64_t> Classifier::Classify0mod8(const std::vector<Matrix>& matrices) const
{
    std::vector<uint64_t> result;

    auto nextQClass = -1;
    auto rowsPositions = GetAllPos(3);
    auto colsPositions = GetAllPos(m_order / 4);
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
            std::cout << "[DEBUG] : [!!!] : New Q class : " << nextQClass << "\n";
        }
        std::vector<Matrix> candidates;
        candidates.push_back(minStartMatrix);
        for (auto next = 0; next < candidates.size(); ++next)
        {
            std::cout << "[DEBUG] : New candidate number " << next + 1 << "\n";

            std::vector<Matrix> candidateAndItsTransposition = {
                    candidates[next],
                    candidates[next].GetTransposed()
            };

            for (const auto& matrix: candidateAndItsTransposition)
            {
                for (const auto& nextRowsPos: rowsPositions)
                {
                    uint64_t lastRowPos;
                    if (CheckRowQuadruple(matrix, nextRowsPos, lastRowPos))
                    {
                        std::cout << "[DEBUG] : Found closed quadruple : "
                                  << nextRowsPos[0] << ", "
                                  << nextRowsPos[1] << ", "
                                  << nextRowsPos[2] << ", "
                                  << lastRowPos << "\n";
                        for (const auto& nextColsPos: colsPositions)
                        {
                            if (CheckRowQClass(matrix, nextColsPos, nextRowsPos))
                            {
                                std::cout << "        : Found quadruple partition\n";
                                auto nextGeneratedMatrix = GetNewMatrix(matrix, nextColsPos, nextRowsPos, lastRowPos);
                                auto minNextGeneratedMatrix = GetMinimalMatrix(nextGeneratedMatrix);
                                auto strGenMatrix = minNextGeneratedMatrix.ToString();
                                if (!m_cachedQClasses[nextQClass].count(strGenMatrix))
                                {
                                    std::cout << "        : Constructed new matrix by switch : "
                                              << candidates.size() << "\n";
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
    }
    return result;
}

std::vector<uint64_t> Classifier::Classify4mod8(const std::vector<Matrix>& matrices) const
{
    // TODO: ...
    return {};
}
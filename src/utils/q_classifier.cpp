#include "q_classifier.h"
#include "mm_finder.h"
#include "debug.h"

#include <iostream>
#include <utility>
#include <algorithm>
#include <bit>

void Classifier::GetNextPos(uint64_t                            count,
                            std::vector<std::vector<uint64_t>>& storage,
                            std::vector<uint64_t>&              tmp,
                            uint64_t                            lhs,
                            uint64_t                            rhs,
                            const std::vector<uint64_t>&        numbers) const
{
    if (count == 0)
    {
        storage.push_back(tmp);
        return;
    }

    for (auto i = lhs; i <= rhs - count; ++i)
    {
        if (numbers.size() == 0)
        {
            tmp.push_back(i);
        }
        else
        {
            tmp.push_back(numbers[i]);
        }
        GetNextPos(count - 1, storage, tmp, i + 1, rhs, numbers);
        tmp.pop_back();
    }
}

std::vector<std::vector<uint64_t>> Classifier::GetAllPos(uint64_t                     count,
                                                         uint64_t                     lhs,
                                                         uint64_t                     rhs,
                                                         const std::vector<uint64_t>& numbers) const
{
    if (count == 0)
    {
        return {};
    }
    std::vector<std::vector<uint64_t>> result;
    std::vector<uint64_t> tmp;
    GetNextPos(count, result, tmp, lhs, rhs, numbers);
    return result;
}

uint64_t Classifier::GetMask(const std::vector<uint64_t>& vec) const
{
    uint64_t result = 0;
    for (auto v: vec)
    {
        result |= (1 << v);
    }
    return result;
}

uint64_t Classifier::MakeMemoKey(const std::vector<uint64_t>& positions) const
{
    uint64_t result = 0;
    for (auto pos: positions)
    {
        result *= 100;
        result += pos;
    }
    return result;
}

bool Classifier::Visited(const std::string& strMatrix, uint64_t& classNum) const
{
    for (auto i = 0; i < m_cachedQClasses.size(); ++i)
    {
        const auto& qClass = m_cachedQClasses[i];
        if (qClass.count(strMatrix))
        {
            classNum = i;
            return true;
        }
    }
    return false;
}

bool Classifier::CheckOneMatrix(const Matrix&                matrix,
                                const std::vector<uint64_t>& rowsPos,
                                const std::vector<uint64_t>& colsPos,
                                uint64_t&                    rowsToNegateMask,
                                uint64_t&                    colsToNegateMask) const
{
    rowsToNegateMask = 0;
    colsToNegateMask = 0;
    std::vector<Row> block;

    // get block of size nxm
    for (auto rowPos: rowsPos)
    {
        block.emplace_back(colsPos.size());
        for (auto i = 0; i < colsPos.size(); ++i)
        {
            auto colPos = colsPos[i];
            block.back() |= (matrix[rowPos][colPos] << i);
        }
    }

    // try to make one matrix from this block
    for (auto i = 0; i < rowsPos.size(); ++i)
    {
        if (block[i][0] == 0)
        {
            rowsToNegateMask |= (1 << rowsPos[i]);
            block[i] = ~block[i];
        }
    }

    for (auto j = 0; j < colsPos.size(); ++j)
    {
        if (block[0][j] == 0)
        {
            colsToNegateMask |= (1 << colsPos[j]);
            for (auto i = 0; i < rowsPos.size(); ++i)
            {
                block[i][j] = !block[i][j];
            }
        }
    }

    // check if success
    for (const auto& row: block)
    {
        if (row.Count() != colsPos.size())
        {
            return false;
        }
    }

    return true;
}

void Classifier::FindBlocks(const Matrix&                             matrix,
                            const std::vector<std::vector<uint64_t>>& rowsPositions,
                            const std::vector<std::vector<uint64_t>>& colsPositions,
                            MatrixMemoStruct&                         memo,
                            bool                                      columns) const
{
    for (const auto& nextRowsPos: rowsPositions)
    {
        std::unordered_set<uint64_t> used;
        std::vector<Classifier::BlockInfo> tmpMemo;

        for (const auto& nextColsPos: colsPositions)
        {
            uint64_t rowsToNegateMask;
            uint64_t colsToNegateMask;

            bool isUsed = false;
            for (auto pos: nextColsPos)
            {
                if (used.count(pos))
                {
                    isUsed = true;
                }
            }
            if (isUsed)
            {
                continue;
            }
            if (CheckOneMatrix(matrix, nextRowsPos, nextColsPos, rowsToNegateMask, colsToNegateMask))
            {
                auto rowMask = GetMask(nextRowsPos);
                auto colMask = GetMask(nextColsPos);
                if (columns)
                {
                    tmpMemo.emplace_back(colMask, rowMask, colsToNegateMask, rowsToNegateMask, nextColsPos, nextRowsPos);
                }
                else
                {
                    tmpMemo.emplace_back(rowMask, colMask, rowsToNegateMask, colsToNegateMask, nextRowsPos, nextColsPos);
                }

                used.insert(nextColsPos.begin(), nextColsPos.end());
            }
        }

        if (tmpMemo.size() != 0)
        {
            auto key = MakeMemoKey(nextRowsPos);
            memo.insert({key, std::move(tmpMemo)});
        }
    }
}

std::vector<Classifier::BlockInfo> Classifier::FindAdditions(const Matrix&                matrix,
                                                             const Classifier::BlockInfo& lastBlockInfo,
                                                             bool                         columns) const
{
    std::vector<Classifier::BlockInfo> result;
    std::vector<uint64_t> candidates;
    auto shapeOfAddition = ((m_order - (m_order % 8)) / 4) % 4;

    if (columns)
    {
        const auto& colsPos = lastBlockInfo.colsPos;
//        const auto& rowsPos = lastBlockInfo.rowsPos;
        auto lastRow = lastBlockInfo.rowsPos.back();

        for (auto i = lastRow + 1; i < m_order; ++i)
        {
            auto cnt = std::popcount((matrix[i].Data() & lastBlockInfo.colsMask) ^ lastBlockInfo.colsNegationMask);
            if (cnt == 4 || cnt == 0)
            {
                candidates.push_back(i);
            }
        }

        if (candidates.size() >= shapeOfAddition)
        {
            auto additionsPositions = GetAllPos(shapeOfAddition, 0, candidates.size(), candidates);
            for (const auto& additionPos: additionsPositions)
            {
                result.emplace_back(0, lastBlockInfo.colsMask, 0, 0, additionPos, std::vector<uint64_t>{});
            }
        }
    }
    else
    {
//        const auto& colsPos = lastBlockInfo.colsPos;
        const auto& rowsPos = lastBlockInfo.rowsPos;
        auto lastCol = lastBlockInfo.colsPos.back();

        for (auto j = lastCol + 1; j < m_order; ++j)
        {
            auto cnt = std::popcount((matrix.ColumnData(j) & lastBlockInfo.rowsMask) ^ lastBlockInfo.rowsNegationMask);
            if (cnt == 4 || cnt == 0)
            {
                candidates.push_back(j);
            }
        }

        if (candidates.size() >= shapeOfAddition)
        {
            auto additionsPositions = GetAllPos(shapeOfAddition, 0, candidates.size(), candidates);
            for (const auto &additionPos: additionsPositions)
            {
                result.emplace_back(0, GetMask(additionPos), 0, 0, lastBlockInfo.rowsPos, std::vector<uint64_t>{});
            }
        }
    }
    return result;
}

// если мы нашли блок 4xn и сделали отрицание, то переходим к следующим строкам или перебираем именно все блоки?
// что если возвращать полученные матрицы, а потом проверять их
void Classifier::RecursiveMatrixBuild(std::vector<uint64_t>&                    indexes,
                                      const std::vector<uint64_t>&              endPerIndex,
                                      uint64_t                                  indexPos,
                                      uint64_t                                  startPos,
                                      const Matrix&                             matrix,
                                      const std::vector<Classifier::BlockInfo>& quadruple,
                                      std::vector<Matrix>&                      result,
                                      bool                                      columns) const
{
    if (indexPos == indexes.size())
    {
        // check if found n / 16 blocks can be represented as 4x(n/4) or (n/4)x4 block
        auto prev = indexes.front();
        for (auto i = 1; i < indexes.size(); ++i)
        {
            if (columns)
            {
                if (quadruple[prev].colsNegationMask != quadruple[indexes[i]].colsNegationMask &&
                    quadruple[prev].colsNegationMask != (quadruple[indexes[i]].colsNegationMask ^ quadruple[indexes[i]].colsMask))
                {
                    if (!(quadruple[prev].rowsPos.back() < quadruple[indexes[i]].rowsPos.front()))
                    {
                        return;
                    }
                }
            }
            else
            {
                if (quadruple[prev].rowsNegationMask != quadruple[indexes[i]].rowsNegationMask &&
                    quadruple[prev].rowsNegationMask != (quadruple[indexes[i]].rowsNegationMask ^ quadruple[indexes[i]].rowsMask))
                {
                    if (!(quadruple[prev].colsPos.back() < quadruple[indexes[i]].colsPos.front()))
                    {
                        return;
                    }
                }
            }
            prev = indexes[i];
        }

        // negate quadruple's block
        auto tmpMatrix{matrix};
        for (auto i: indexes)
        {
            const auto& rowsPos = quadruple[i].rowsPos;
            auto colsMask       = quadruple[i].colsMask;
            for (auto nextRow: rowsPos)
            {
                auto row = tmpMatrix[nextRow];
                tmpMatrix[nextRow] = (row & ~colsMask) | (~row & colsMask);
            }
        }

        auto shapeOfAddition = ((m_order - (m_order % 8)) / 4) % 4;
        if (shapeOfAddition == 0)
        {
            result.push_back(std::move(tmpMatrix));
        }
        else
        {
            // find possible additions to block 4xm or mx4
            // find all additions and add for loop
            std::vector<Classifier::BlockInfo> additionsInfo = FindAdditions(matrix, quadruple[indexes.back()], columns);
            // negate additions
            for (const auto& additionInfo: additionsInfo)
            {
                auto newMatrix{tmpMatrix};

                for (auto nextRow: additionInfo.rowsPos)
                {
                    auto row = newMatrix[nextRow];
                    newMatrix[nextRow] = (row & ~additionInfo.colsMask) | (~row & additionInfo.colsMask);
                }

                // add new generated matrix to result
                result.push_back(std::move(newMatrix));
            }
        }
    }
    else
    {
        for (indexes[indexPos] = startPos; indexes[indexPos] != endPerIndex[indexPos]; ++indexes[indexPos])
        {
            RecursiveMatrixBuild(indexes, endPerIndex,
                                 indexPos + 1, indexes[indexPos] + 1,
                                 matrix, quadruple, result, columns);
        }
    }
}

std::vector<uint64_t> Classifier::Classify(const std::vector<Matrix>& matrices) const
{
    std::vector<uint64_t> result;

    auto nextQClass        = -1;
    auto countOfBlocks     = m_order / 16;

    auto blockPositions    = GetAllPos(4, 0, m_order);

    for (const auto& startMatrix: matrices)
    {
        auto minStartMatrix = GetMinimalMatrix(startMatrix, m_precalculatedMinMatrices);
        auto strStartMatrix = minStartMatrix.ToString();
        uint64_t possibleClass;

        if (m_cachedQClasses.size() > 0 && Visited(strStartMatrix, possibleClass))
        {
            std::cerr << "\n[DEBUG] : Already visited, Q class of matrix is " << possibleClass + 1 << "\n";
            std::cerr << "[DEBUG] : Minimal matrix:\n";
            DEBUG_PRINT_MATRIX(minStartMatrix);
            result.push_back(possibleClass);
            continue;
        }
        else
        {
            ++nextQClass;
            result.push_back(nextQClass);
            m_cachedQClasses.emplace_back();
            m_cachedQClasses.back().insert(strStartMatrix);
            m_precalculatedMinMatrices.insert(strStartMatrix);
            std::cerr << "\n[DEBUG] : New Q class " << nextQClass + 1 << "\n";
            std::cerr << "[DEBUG] : Minimal matrix:\n";
            DEBUG_PRINT_MATRIX(minStartMatrix);
        }

        std::vector<Matrix> candidates;
        candidates.push_back(minStartMatrix);

        std::unordered_set<std::string> candidatesSet;
        candidatesSet.insert(candidates.front().ToString());

        for (auto next = 0; next < candidates.size(); ++next)
        {
            auto matrix = candidates[next];
            std::cerr << "[DEBUG] : [ QClass "
                      << nextQClass + 1
                      << " ] : Start with new candidate number "
                      << next + 1 << "\n";

            MemoContext blocksMemo;

            FindBlocks(matrix, blockPositions, blockPositions, blocksMemo.rowBlocksMemo, false);
            FindBlocks(matrix.RowsAreCols(), blockPositions, blockPositions, blocksMemo.colBlocksMemo, true);

            if (m_order % 8 == 0)
            {
                // handle rows
                for (const auto& [rowsKey, quadruple]: blocksMemo.rowBlocksMemo)
                {
                    std::vector<uint64_t> indexes(m_order / 16, 0);
                    std::vector<uint64_t> endPerIndex(m_order / 16, quadruple.size());
                    std::vector<Matrix>   newMatrices;

                    RecursiveMatrixBuild(indexes, endPerIndex, 0, 0, matrix, quadruple, newMatrices, false);

                    for (const auto& newMatrix: newMatrices)
                    {
                        // check if generated matrix is Hadamard
                        if (!newMatrix.IsHadamard())
                        {
                            continue;
                        }

                        // add new matrix to candidates or reject
                        auto minMatrix = GetMinimalMatrix(newMatrix, m_precalculatedMinMatrices);
                        auto strMatrix = minMatrix.ToString();

                        if (!m_cachedQClasses.back().count(strMatrix) && !candidatesSet.count(strMatrix))
                        {
                            m_cachedQClasses.back().insert(strMatrix);
                            m_precalculatedMinMatrices.insert(strMatrix);
                            candidates.push_back(minMatrix);
                            candidatesSet.insert(strMatrix);
                            std::cerr << "[DEBUG] : [ QClass "
                                      << nextQClass + 1
                                      << " ] : Inserted new candidate number "
                                      << candidates.size() << "\n";
                            DEBUG_PRINT_MATRIX_MASK(matrix, newMatrix);
                        }
                    }
                }
                // handle columns
                for (const auto& [colsKey, quadruple]: blocksMemo.colBlocksMemo)
                {
                    std::vector<uint64_t> indexes(m_order / 16, 0);
                    std::vector<uint64_t> endPerIndex(m_order / 16, quadruple.size());
                    std::vector<Matrix>   newMatrices;

                    RecursiveMatrixBuild(indexes, endPerIndex, 0, 0, matrix, quadruple, newMatrices, true);

                    for (const auto& newMatrix: newMatrices)
                    {
                        // check if generated matrix is Hadamard
                        if (!newMatrix.IsHadamard())
                        {
                            continue;
                        }

                        // add new matrix to candidates or reject
                        auto minMatrix = GetMinimalMatrix(newMatrix, m_precalculatedMinMatrices);
                        auto strMatrix = minMatrix.ToString();

                        if (!m_cachedQClasses.back().count(strMatrix) && !candidatesSet.count(strMatrix))
                        {
                            m_cachedQClasses.back().insert(strMatrix);
                            m_precalculatedMinMatrices.insert(strMatrix);
                            candidates.push_back(minMatrix);
                            candidatesSet.insert(strMatrix);
                            std::cerr << "[DEBUG] : [ QClass "
                                      << nextQClass + 1
                                      << " ] : Inserted new candidate number "
                                      << candidates.size() << "\n";
                            DEBUG_PRINT_MATRIX_MASK(matrix, newMatrix);
                        }
                    }
                }
            }
            else
            {
                for (const auto& [rowsKey, rowsQuadruple]: blocksMemo.rowBlocksMemo)
                {
                    std::vector<uint64_t> indexes(m_order / 16, 0);
                    std::vector<uint64_t> endPerIndex(m_order / 16, rowsQuadruple.size());
                    std::vector<Matrix>   newTmpMatrices;

                    RecursiveMatrixBuild(indexes, endPerIndex, 0, 0, matrix, rowsQuadruple, newTmpMatrices, false);

                    for (const auto& newTmpMatrix: newTmpMatrices)
                    {
                        for (const auto& [colsKey, colsQuadruple]: blocksMemo.colBlocksMemo)
                        {
                            std::vector<uint64_t> indexes(m_order / 16, 0);
                            std::vector<uint64_t> endPerIndex(m_order / 16, colsQuadruple.size());
                            std::vector<Matrix>   newMatrices;

                            RecursiveMatrixBuild(indexes, endPerIndex, 0, 0, newTmpMatrix, colsQuadruple, newMatrices, true);

                            for (const auto& newMatrix: newMatrices)
                            {
                                // check if generated matrix is Hadamard
                                if (!newMatrix.IsHadamard())
                                {
                                    continue;
                                }

                                // add new matrix to candidates or reject
                                auto minMatrix = GetMinimalMatrix(newMatrix, m_precalculatedMinMatrices);
                                auto strMatrix = minMatrix.ToString();

                                if (!m_cachedQClasses.back().count(strMatrix) && !candidatesSet.count(strMatrix))
                                {
                                    m_cachedQClasses.back().insert(strMatrix);
                                    m_precalculatedMinMatrices.insert(strMatrix);
                                    candidates.push_back(minMatrix);
                                    candidatesSet.insert(strMatrix);
                                    std::cerr << "[DEBUG] : [ QClass "
                                              << nextQClass + 1
                                              << " ] : Inserted new candidate number "
                                              << candidates.size() << "\n";
                                    DEBUG_PRINT_MATRIX_MASK(matrix, newMatrix);
//                                    DEBUG_PRINT_MATRIX_MASK(matrix, minMatrix);
                                    DEBUG_PRINT_MATRIX(minMatrix);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}
#include "q_classifier.h"
#include "mm_finder.h"
#include "debug.h"

#include <iostream>
#include <utility>
#include <algorithm>

void Classifier::GetNextPos(uint64_t                            count,
                            std::vector<std::vector<uint64_t>>& storage,
                            std::vector<uint64_t>&              tmp,
                            uint64_t                            lhs,
                            uint64_t                            rhs) const
{
    if (count == 0)
    {
        storage.push_back(tmp);
        return;
    }

    for (auto i = lhs; i <= rhs - count; ++i)
    {
        tmp.push_back(i);
        GetNextPos(count - 1, storage, tmp, i + 1, rhs);
        tmp.pop_back();
    }
}

std::vector<std::vector<uint64_t>> Classifier::GetAllPos(uint64_t count, uint64_t lhs, uint64_t rhs) const
{
    if (count == 0)
    {
        return {};
    }
    std::vector<std::vector<uint64_t>> result;
    std::vector<uint64_t> tmp;
    GetNextPos(count, result, tmp, lhs, rhs);
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

bool Classifier::Complements(const Classifier::BlockInfo& lhs, const Classifier::BlockInfo& rhs) const
{
    auto lhsColsMask = lhs.colsMask;
    auto rhsColsMask = rhs.colsMask;

    auto lowestRhsBit  = rhsColsMask & (-rhsColsMask);
    auto highestLhsBit = 1;
    while (lhsColsMask >>= 1)
    {
        highestLhsBit <<= 1;
    }

    return highestLhsBit < lowestRhsBit;
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

void Classifier::FindBlocksRecursive(const Matrix&                             matrix,
                                     const std::vector<uint64_t>&              nextRowsPos,
                                     const std::vector<std::vector<uint64_t>>& combinations,
                                     uint64_t                                  prevCombinationIdx,
                                     uint64_t                                  leftBorder,
                                     std::vector<Classifier::BlockInfo>&       rowMemo,
                                     MemoContext&                              memo) const
{
    if (leftBorder >= m_order)
    {
        return;
    }
    auto nextCombinationIdx = prevCombinationIdx;
    while (nextCombinationIdx < combinations.size()
           && leftBorder != combinations[nextCombinationIdx].front())
    {
        ++nextCombinationIdx;
    }
    for (auto i = nextCombinationIdx; i < combinations.size(); ++i)
    {
        const auto& nextColsPos = combinations[i];

        uint64_t rowsToNegateMask;
        uint64_t colsToNegateMask;

        if (CheckOneMatrix(matrix, nextRowsPos, nextColsPos, rowsToNegateMask, colsToNegateMask))
        {
            auto rowMask = GetMask(nextRowsPos);
            auto colMask = GetMask(nextColsPos);
            rowMemo.emplace_back(rowMask, colMask, rowsToNegateMask, colsToNegateMask);

            auto key = MakeMemoKey(nextColsPos);
            memo.colBlocksMemo[key].emplace_back(rowMask, colMask, rowsToNegateMask, colsToNegateMask);

            FindBlocksRecursive(matrix, nextRowsPos, combinations, i + 1, nextColsPos.back() + 1, rowMemo, memo);
            return;
        }
    }
}

void Classifier::FindBlocks(const Matrix&                             matrix,
                            const std::vector<std::vector<uint64_t>>& rowsPositions,
                            const std::vector<std::vector<uint64_t>>& colsPositions,
                            MemoContext&                              memo) const
{
    for (const auto& nextRowsPos: rowsPositions)
    {
        std::vector<Classifier::BlockInfo> tmpRowMemo;

        FindBlocksRecursive(matrix, nextRowsPos, colsPositions, 0, 0, tmpRowMemo, memo);

        if (tmpRowMemo.size() != 0)
        {
            auto key = MakeMemoKey(nextRowsPos);
            memo.rowBlocksMemo.insert({key, std::move(tmpRowMemo)});
        }
    }
}

std::vector<Classifier::BlockInfo> Classifier::FindAdditions(const Classifier::BlockInfo&              lastBlockInfo,
                                                             const std::vector<Classifier::BlockInfo>& additions,
                                                             bool                                      columns) const
{
    std::vector<Classifier::BlockInfo> result;
    for (const auto& addition: additions)
    {
        if (Complements(lastBlockInfo, addition))
        {
            if ((columns && lastBlockInfo.colsNegationMask == addition.colsNegationMask)
                || (!columns && lastBlockInfo.rowsNegationMask == addition.rowsNegationMask))
            {
                result.push_back(addition);
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
                                      const std::vector<Classifier::BlockInfo>& additions,
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
                if (quadruple[prev].colsNegationMask != quadruple[indexes[i]].colsNegationMask)
                {
                    return;
                }
            }
            else
            {
                if (quadruple[prev].rowsNegationMask != quadruple[indexes[i]].rowsNegationMask)
                {
                    return;
                }
            }
            prev = indexes[i];
        }

        // find possible additions to block 4xm or mx4
        // find all additions and add for loop
        std::vector<Classifier::BlockInfo> additionsInfo = FindAdditions(quadruple[indexes.back()], additions, columns);

        // negate quadruple's block
        auto tmpMatrix{matrix};
        std::vector<uint64_t> rowsPos;
        for (auto nextPos = 0; nextPos < m_order; ++nextPos)
        {
            if ((1 << nextPos) & quadruple.front().rowsMask)
            {
                rowsPos.push_back(nextPos);
            }
        }

        for (auto i: indexes)
        {
            const auto& info = quadruple[i];
            auto colsMask    = info.colsMask;

            for (auto nextRow: rowsPos)
            {
                auto row = tmpMatrix[nextRow];
                tmpMatrix[nextRow] = (row & ~colsMask) | (~row & colsMask);
            }
        }

        if (additions.size() == 0)
        {
            result.push_back(std::move(tmpMatrix));
        }
        else
        {
            // negate additions
            for (const auto& additionInfo: additionsInfo)
            {
                auto newMatrix{tmpMatrix};

                for (auto nextRow: rowsPos)
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
                                 matrix, quadruple, additions, result);
        }
    }
}

std::vector<uint64_t> Classifier::Classify(const std::vector<Matrix>& matrices) const
{
    std::vector<uint64_t> result;

    auto nextQClass        = -1;
    auto countOfBlocks     = m_order / 16;
    auto shapeOfAddition   = ((m_order - (m_order % 8)) / 4) % 4;

    auto blockPositions    = GetAllPos(4, 0, m_order);
    auto additionPositions = GetAllPos(shapeOfAddition, 0, m_order);

    for (const auto& startMatrix: matrices)
    {
        auto minStartMatrix = GetMinimalMatrix(startMatrix, m_precalculatedMinMatrices);
        auto strMatrix      = minStartMatrix.ToString();
        uint64_t possibleClass;

        if (m_cachedQClasses.size() > 0 && Visited(strMatrix, possibleClass))
        {
            std::cout << "[DEBUG] : Already visited, Q class of matrix is " << possibleClass << "\n";
            std::cout << "[DEBUG] : Minimal matrix:\n";
            DEBUG_PRINT_MATRIX(minStartMatrix);
            result.push_back(possibleClass);
            continue;
        }
        else
        {
            ++nextQClass;
            result.push_back(nextQClass);
            m_cachedQClasses.emplace_back();
            m_cachedQClasses.back().insert(strMatrix);
            // TODO: remove after
            m_precalculatedMinMatrices.insert(strMatrix);
            std::cout << "[DEBUG] : New Q class " << nextQClass + 1 << "\n";
            std::cout << "[DEBUG] : Minimal matrix:\n";
            DEBUG_PRINT_MATRIX(minStartMatrix);
        }

        std::vector<Matrix> candidates;
        candidates.push_back(minStartMatrix);

        for (auto next = 0; next < candidates.size(); ++next)
        {
            auto matrix = candidates[next];
            std::cout << "[DEBUG] : Start with new candidate number " << next + 1 << "\n";

            MemoContext blocksMemo;
            MemoContext additionsMemo;
            MemoContext trAdditionsMemo;

            FindBlocks(matrix, blockPositions, blockPositions, blocksMemo);
            if (shapeOfAddition != 0)
            {
                FindBlocks(matrix, blockPositions, additionPositions, additionsMemo);
                FindBlocks(matrix, additionPositions, blockPositions, trAdditionsMemo);
            }

            if (m_order % 8 == 0)
            {
                // handle rows
                for (const auto& [rowsKey, quadruple]: blocksMemo.rowBlocksMemo)
                {
                    std::vector<uint64_t> indexes(m_order / 16, 0);
                    std::vector<uint64_t> endPerIndex(m_order / 16, quadruple.size());
                    std::vector<Matrix>   newMatrices;
                    const auto& additions = additionsMemo.rowBlocksMemo[rowsKey];

                    RecursiveMatrixBuild(indexes, endPerIndex, 0, 0, matrix, quadruple, additions, newMatrices);

                    for (const auto& newMatrix: newMatrices)
                    {
                        // check if generated matrix is Hadamard
                        if (!newMatrix.IsHadamard())
                        {
                            continue;
                        }

                        // add new matrix to candidates or reject
                        // TODO: remove after
                        auto minMatrix = GetMinimalMatrix(newMatrix, m_precalculatedMinMatrices);
                        auto strMatrix = minMatrix.ToString();

                        if (!m_cachedQClasses.back().count(strMatrix))
                        {
                            m_cachedQClasses.back().insert(strMatrix);
                            // TODO: remove after
                            m_precalculatedMinMatrices.insert(strMatrix);
                            candidates.push_back(newMatrix);
                            std::cout << "[DEBUG] : Inserted new candidate number " << candidates.size() << "\n";
                        }
                    }
                }
                // handle columns
                for (const auto& [colsKey, quadruple]: blocksMemo.colBlocksMemo)
                {
                    std::vector<uint64_t> indexes(m_order / 16, 0);
                    std::vector<uint64_t> endPerIndex(m_order / 16, quadruple.size());
                    std::vector<Matrix>   newMatrices;
                    const auto& additions = trAdditionsMemo.colBlocksMemo[colsKey];

                    RecursiveMatrixBuild(indexes, endPerIndex, 0, 0, matrix, quadruple, additions, newMatrices);

                    for (const auto& newMatrix: newMatrices)
                    {
                        // check if generated matrix is Hadamard
                        if (!newMatrix.IsHadamard())
                        {
                            continue;
                        }

                        // add new matrix to candidates or reject
                        // TODO: remove after
                        auto minMatrix = GetMinimalMatrix(newMatrix, m_precalculatedMinMatrices);
                        auto strMatrix = minMatrix.ToString();

                        if (!m_cachedQClasses.back().count(strMatrix))
                        {
                            m_cachedQClasses.back().insert(strMatrix);
                            // TODO: remove after
                            m_precalculatedMinMatrices.insert(strMatrix);
                            candidates.push_back(newMatrix);
                            std::cout << "[DEBUG] : Inserted new candidate number " << candidates.size() << "\n";
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
                    const auto& rowsAdditions = additionsMemo.rowBlocksMemo[rowsKey];

                    RecursiveMatrixBuild(indexes, endPerIndex, 0, 0, matrix, rowsQuadruple, rowsAdditions, newTmpMatrices);

                    for (const auto& newTmpMatrix: newTmpMatrices)
                    {
                        for (const auto& [colsKey, colsQuadruple]: blocksMemo.colBlocksMemo)
                        {
                            std::vector<uint64_t> indexes(m_order / 16, 0);
                            std::vector<uint64_t> endPerIndex(m_order / 16, colsQuadruple.size());
                            std::vector<Matrix>   newMatrices;
                            const auto& colsAdditions = trAdditionsMemo.colBlocksMemo[colsKey];

                            RecursiveMatrixBuild(indexes, endPerIndex, 0, 0, newTmpMatrix, colsQuadruple, colsAdditions, newMatrices);

                            for (const auto& newMatrix: newMatrices)
                            {
                                // check if generated matrix is Hadamard
                                if (!newMatrix.IsHadamard())
                                {
                                    continue;
                                }

                                // add new matrix to candidates or reject
                                // TODO: remove after
                                auto minMatrix = GetMinimalMatrix(newMatrix, m_precalculatedMinMatrices);
                                auto strMatrix = minMatrix.ToString();

                                if (!m_cachedQClasses.back().count(strMatrix))
                                {
                                    m_cachedQClasses.back().insert(strMatrix);
                                    // TODO: remove after
                                    m_precalculatedMinMatrices.insert(strMatrix);
                                    candidates.push_back(newMatrix);
                                    std::cout << "[DEBUG] : Inserted new candidate number " << candidates.size() << "\n";
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
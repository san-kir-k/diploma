#pragma once

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>
#include <utility>

#include "matrix.h"
#include "row.h"

class Classifier
{
public:
    explicit Classifier() = default;
    explicit Classifier(uint64_t order)
        : m_order(order)
        , m_cachedQClasses()
        , m_precalculatedMinMatrices()
    {
    }
    explicit Classifier(uint64_t order, std::unordered_set<std::string>&& precalculated)
        : m_order(order)
        , m_cachedQClasses()
        , m_precalculatedMinMatrices(std::move(precalculated))
    {
    }

    ~Classifier() = default;

    std::vector<uint64_t> Classify(const std::vector<Matrix>& matrices) const;

private:
    struct BlockInfo
    {
        uint64_t rowsMask;
        uint64_t colsMask;
        uint64_t rowsNegationMask;
        uint64_t colsNegationMask;
        std::vector<uint64_t> rowsPos;
        std::vector<uint64_t> colsPos;

        inline BlockInfo() = default;
        inline BlockInfo(uint64_t              rowsMask,
                         uint64_t              colsMask,
                         uint64_t              rowsNegationMask,
                         uint64_t              colsNegationMask,
                         std::vector<uint64_t> rowsPos,
                         std::vector<uint64_t> colsPos)
            : rowsMask(rowsMask)
            , colsMask(colsMask)
            , rowsNegationMask(rowsNegationMask)
            , colsNegationMask(colsNegationMask)
            , rowsPos(rowsPos)
            , colsPos(colsPos)
        {
        }
    };

    using MatrixMemoStruct = std::unordered_map<uint64_t, std::vector<BlockInfo>>;

    struct MemoContext
    {
        MatrixMemoStruct colBlocksMemo;
        MatrixMemoStruct rowBlocksMemo;
    };

private:
    void GetNextPos(uint64_t                            count,
                    std::vector<std::vector<uint64_t>>& storage,
                    std::vector<uint64_t>&              tmp,
                    uint64_t                            lhs,
                    uint64_t                            rhs,
                    const std::vector<uint64_t>&        numbers) const;
    std::vector<std::vector<uint64_t>> GetAllPos(uint64_t count,
                                                 uint64_t lhs,
                                                 uint64_t rhs,
                                                 const std::vector<uint64_t>& numbers = {}) const;

    uint64_t GetMask(const std::vector<uint64_t>& vec) const;

    bool Visited(const std::string& strMatrix, uint64_t& classNum) const;

    bool CheckOneMatrix(const Matrix&                matrix,
                        const std::vector<uint64_t>& rowsPos,
                        const std::vector<uint64_t>& colsPos,
                        uint64_t&                    rowsToNegateMask,
                        uint64_t&                    colsToNegateMask) const;

    uint64_t MakeMemoKey(const std::vector<uint64_t>& positions) const;

    void FindBlocks(const Matrix&                             matrix,
                    const std::vector<std::vector<uint64_t>>& rowsPositions,
                    const std::vector<std::vector<uint64_t>>& colsPositions,
                    MatrixMemoStruct&                         memo,
                    bool                                      columns) const;

    std::vector<BlockInfo> FindAdditions(const Matrix&    matrix,
                                         const BlockInfo& lastBlockInfo,
                                         bool             columns) const;

    void RecursiveMatrixBuild(std::vector<uint64_t>&                    indexes,
                              const std::vector<uint64_t>&              endPerIndex,
                              uint64_t                                  indexPos,
                              uint64_t                                  startPos,
                              const Matrix&                             matrix,
                              const std::vector<Classifier::BlockInfo>& quadruple,
                              std::vector<Matrix>&                      result,
                              bool                                      columns) const;

private:
    uint64_t                                     m_order;
    mutable
    std::vector<std::unordered_set<std::string>> m_cachedQClasses;
    // TODO: remove after
    mutable
    std::unordered_set<std::string>              m_precalculatedMinMatrices;
};
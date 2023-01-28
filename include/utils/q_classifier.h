#pragma once

#include <unordered_set>
#include <string>
#include <vector>

#include "matrix.h"
#include "row.h"

class Classifier
{
public:
    explicit Classifier(uint64_t order): m_order(order), m_cachedQClasses() {}

    ~Classifier() = default;

    std::vector<uint64_t> Classify(const std::vector<Matrix>& matrices) const;

private:
    std::vector<uint64_t> Classify0mod8(const std::vector<Matrix>& matrices) const;

    std::vector<uint64_t> Classify4mod8(const std::vector<Matrix>& matrices) const;

    void GetNextRowsPos(uint64_t offset, uint64_t count,
                        std::vector<std::vector<uint64_t>>& storage,
                        std::vector<uint64_t>& tmp) const;
    std::vector<std::vector<uint64_t>> GetAllRowsPos(uint64_t count) const;

    bool CheckQuadruple(const Matrix& matrix,
                        const std::vector<uint64_t>& colsPos,
                        const std::vector<uint64_t>& rowsPos) const;

    Matrix GetNewMatrix(const Matrix& matrix,
                        const std::vector<uint64_t>& colsPos,
                        const std::vector<uint64_t>& rowsPos) const;

private:
    uint64_t                                     m_order;
    mutable
    std::vector<std::unordered_set<std::string>> m_cachedQClasses;
};
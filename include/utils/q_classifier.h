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

    void GetNextPos(uint64_t offset, uint64_t count,
                        std::vector<std::vector<uint64_t>>& storage,
                        std::vector<uint64_t>& tmp) const;
    std::vector<std::vector<uint64_t>> GetAllPos(uint64_t count) const;

    bool CheckRowQuadruple(const Matrix& matrix,
                           const std::vector<uint64_t>& rowsPos) const;
    bool CheckColQuadruple(const Matrix& matrix,
                           const std::vector<uint64_t>& rowsPos) const;
    bool CheckRowQClass(const Matrix& matrix,
                        const std::vector<uint64_t>& colsPos,
                        const std::vector<uint64_t>& rowsPos) const;
    bool CheckColQClass(const Matrix& matrix,
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
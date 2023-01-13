#pragma once
#include "bucket.h"

// не очень полезный по итогу класс
class HadamardMatrixBuilder
{
public:
    explicit HadamardMatrixBuilder(uint64_t order, bool countOnly = false);

    void CountMatrices() const;

    void PrintMatrices() const;

    ~HadamardMatrixBuilder() = default;

private:
    Bucket m_bucket;
};
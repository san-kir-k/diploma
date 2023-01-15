#pragma once

#include "matrix_printer.h"
#include "bucket.h"
#include "mode.h"

class HadamardMatrix
{
public:
    explicit HadamardMatrix(uint64_t order, Mode mode = Mode::NORMAL);

    void GetResult() const;

    static void FindMinimalMatrix(const std::string& filename, uint64_t num);

    ~HadamardMatrix() = default;

private:
    uint64_t      m_order;
    Bucket        m_bucket;
    Mode          m_mode;
    MatrixPrinter m_printer;
};
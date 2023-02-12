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

    static void FindMinimalMatrices(const std::string& filename);

    static void FindQClasses(const std::string& dirname, bool singleFile = false);

    ~HadamardMatrix() = default;

private:
    [[maybe_unused]] static bool GetMatrixFromFileCore(std::ifstream& in, uint64_t order, Matrix& m);
    static Matrix GetMatrixFromFile(const std::string& filename);
    static std::vector<Matrix> GetMatricesFromFile(const std::string& filename);

private:
    uint64_t      m_order;
    Bucket        m_bucket;
    Mode          m_mode;
    MatrixPrinter m_printer;
};
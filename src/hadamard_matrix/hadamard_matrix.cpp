#include "hadamard_matrix.h"

HadamardMatrixBuilder::HadamardMatrixBuilder(uint64_t order, bool countOnly)
    : m_bucket(order, countOnly)
{
    m_bucket.GenerateMatrix();
}

void HadamardMatrixBuilder::CountMatrices() const
{
    m_bucket.CountFoundMatrices();
}

void HadamardMatrixBuilder::PrintMatrices() const
{
    m_bucket.PrintFoundMatrices();
}
#include "row.h"

RowProxy Row::operator[](uint64_t pos)
{
    assert(pos < m_size);
    return RowProxy{*this, pos};
}
#include "row.h"

RowProxy Row::operator[](uint64_t pos)
{
    return RowProxy{*this, pos};
}
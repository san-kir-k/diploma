#pragma once
#include <cstdint>
#include <cassert>
#include <bit>

class RowProxy;

class Row
{
public:
    constexpr explicit Row(): m_size(0), m_data(0) {}
    constexpr explicit Row(uint64_t size): m_size(size), m_data(0) {}
    constexpr explicit Row(uint64_t size, uint64_t data): m_size(size), m_data(data) {}

    constexpr Row(const Row&) = default;
    constexpr Row& operator=(const Row&) = default;

    constexpr Row(Row&&) = default;
    constexpr Row& operator=(Row&&) = default;

    constexpr ~Row() = default;

    inline constexpr uint64_t Data() const { return m_data; }
    inline constexpr uint64_t& Data() { return m_data; }

    inline constexpr uint64_t Size() const { return m_size; }

    inline constexpr uint64_t Count() const
    {
        return std::popcount(m_data);
    }

    inline constexpr bool operator[](uint64_t pos) const
    {
        assert(pos < m_size);
        return m_data & (1 << pos);
    }
    RowProxy operator[](uint64_t pos);

    inline constexpr bool operator==(const Row& other) const
    {
        return m_size == other.m_size && m_data == other.m_data;
    }
    inline constexpr bool operator!=(const Row& other) const
    {
        return !(*this == other);
    }
    inline constexpr bool operator<(const Row& other) const
    {
        return m_size == other.m_size && m_data < other.m_data;
    }
    inline constexpr bool operator>(const Row& other) const
    {
        return m_size == other.m_size && m_data > other.m_data;
    }

    inline constexpr Row& operator^=(const Row& other)
    {
        m_data ^= other.m_data;
        return *this;
    }
    inline constexpr Row& operator|=(const Row& other)
    {
        m_data |= other.m_data;
        return *this;
    }
    inline constexpr Row& operator|=(uint64_t data)
    {
        m_data |= data;
        return *this;
    }
    inline constexpr Row& operator&=(const Row& other)
    {
        m_data &= other.m_data;
        return *this;
    }
    inline constexpr Row& operator&=(uint64_t data)
    {
        m_data &= data;
        return *this;
    }
    inline constexpr Row operator~() const
    {
        return Row{m_size, (~m_data) & ((1 << m_size) - 1)};
    }

    inline constexpr Row operator<<(uint64_t pos) const
    {
        assert(pos < m_size);
        return Row{m_size, m_data << pos};
    }
    inline constexpr Row& operator<<=(uint64_t pos)
    {
        assert(pos < m_size);
        m_data <<= pos;
        return *this;
    }
    inline constexpr Row operator>>(uint64_t pos) const
    {
        assert(pos < m_size);
        return Row{m_size, m_data >> pos};
    }
    inline constexpr Row& operator>>=(uint64_t pos)
    {
        assert(pos < m_size);
        m_data >>= pos;
        return *this;
    }

private:
    uint64_t m_size;
    uint64_t m_data;
};

inline constexpr Row operator^(const Row& lhs, const Row& rhs)
{
    Row res = lhs;
    res ^= rhs;
    return res;
}

inline constexpr Row operator|(const Row& lhs, const Row& rhs)
{
    Row res = lhs;
    res |= rhs;
    return res;
}

inline constexpr Row operator|(const Row& lhs, uint64_t rhsData)
{
    Row res = lhs;
    res |= rhsData;
    return res;
}

inline constexpr Row operator&(const Row& lhs, const Row& rhs)
{
    Row res = lhs;
    res &= rhs;
    return res;
}

inline constexpr Row operator&(const Row& lhs, uint64_t rhsData)
{
    Row res = lhs;
    res &= rhsData;
    return res;
}

inline constexpr bool AreOrthogonal(const Row& lhs, const Row& rhs)
{
    return (lhs ^ rhs).Count() == (lhs.Size() / 2);
}

class RowProxy
{
public:
    RowProxy(Row& row, uint64_t pos): m_row(row), m_pos(pos) {}
    RowProxy(const RowProxy&) = default;

    ~RowProxy() = default;

    inline operator bool() const { return m_row.Data() & (1 << m_pos); }

    inline constexpr RowProxy& operator=(bool val)
    {
        if (val)
        {
            m_row |= (1 << m_pos);
        }
        else
        {
            m_row &= ~(1 << m_pos);
        }
        return *this;
    }

    inline constexpr RowProxy& operator=(const RowProxy& other)
    {
        return *this = bool(other);
    }

    friend void swap(RowProxy lhs, RowProxy rhs)
    {
        bool tmp = lhs;
        lhs = rhs;
        rhs = tmp;
    }

private:
    Row&     m_row;
    uint64_t m_pos;
};
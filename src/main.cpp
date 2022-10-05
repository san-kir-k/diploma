//
// Created by AK on 05.10.2022.
//

/**
 * Берем строчку (111…111), ставим на первое месте, относительно нее сделать множество,
 * это множество назовем корзиной (оно ортогонально певрой строке),
 * из этой корзины выбираем строку самую первую и размещаем на второй позиции,
 * потом строим множество, в котором все строки ортогональны первым двум строкам.
 *
 * Предыдущую корзину и выбранную строку и просеиваем корзину, оставляя только те,
 * которые ортогональны выбранной. Если корзина опустела до конца,
 * то возвращаемся на шаг назад, то текущую строку менять на следующую строку из корзины и продолжить алгоритм.
 */

#include <iostream>
#include <cstdint>
#include <vector>
#include <cassert>
#include <bitset>

class Bucket
{
public:
    using Row = uint64_t;

public:
    explicit Bucket(int order)
        : m_order(order)
    {
        assert(order <= 64 && order >= 1);
        Row first = 0;
        for (uint64_t i = 0; i < order; ++i)
        {
            first |= (1ULL << i);
        }
        m_completedRows.push_back(first);
    }

    ~Bucket() = default;

private:
    int              m_order;
    std::vector<Row> m_completedRows;
};

class HadamardMatrixBuilder
{
public:
    explicit HadamardMatrixBuilder(int order)
        : m_order(order)
        , m_bucket(order)
    {
    }

    ~HadamardMatrixBuilder() = default;

private:
    int    m_order;
    Bucket m_bucket;
};

int main()
{
    HadamardMatrixBuilder(63);
    std::cout << sizeof(std::bitset<64>(0)) << ", " << sizeof(std::bitset<8>(0)) << "\n";
    std::cout << "Hello diploma!\n";
    return 0;
}
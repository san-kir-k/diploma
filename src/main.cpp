#include <iostream>
#include <cstring>

#include "hadamard_matrix.h"
#include "matrix_printer.h"
#include "bucket.h"

int main(int argc, char** argv)
{
    size_t n = 1;
    while (n <= 12)
    {
        // если запускать без COUNT_ONLY, то программа будет выводить матрицы в файлики
        // а если с COUNT_ONLY, то только подсчитает их число
        if (argc > 1 && std::strcmp(argv[1], "COUNT_ONLY") == 0)
        {
            HadamardMatrixBuilder b(n, true);
//            b.CountMatrices();
        }
        else
        {
            HadamardMatrixBuilder b(n);
//            b.PrintMatrices();
        }
        if (n < 4)
        {
            n *= 2;
        }
        else
        {
            n += 4;
        }
    }
    return 0;
}
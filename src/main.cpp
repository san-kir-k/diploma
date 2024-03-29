#include <cassert>
#include <string>

#include "hadamard_matrix.h"
#include "matrix_printer.h"
#include "mode.h"
#include "cli_args_parser.h"

int main(int argc, char** argv)
{
    CLIArgsParser parser{argc, argv};

    if (parser.IsOptionExists("-gen"))
    {
        const auto& strMode = parser.GetOption("-gen");
        Mode mode = Mode::NORMAL;
        if (strMode == "COUNT_ONLY")
        {
            mode = Mode::COUNT_ONLY;
        }
        else if (strMode == "UNIQUE_ONLY")
        {
            mode = Mode::UNIQUE_ONLY;
        }

        if (!parser.IsOptionExists("-num"))
        {
            assert(false);
        }
        auto maxN = std::stoi(parser.GetOption("-num"));
        auto n = 1;

        while (n <= maxN)
        {
            HadamardMatrix b(n, mode);
            b.GetResult();

            if (n < 4)
            {
                n *= 2;
            }
            else
            {
                n += 4;
            }
        }

    }
    else if (parser.IsOptionExists("-find_mm"))
    {
        const auto& filename = parser.GetOption("-find_mm");
        HadamardMatrix::FindMinimalMatrix(filename, 0);
    }
    else
    {
        assert(false);
    }
    return 0;
}
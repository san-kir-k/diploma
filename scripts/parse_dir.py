import os
import sys

if __name__ == "__main__":
    walk_dir = sys.argv[1]
    outfile = sys.argv[2]
    dim = sys.argv[3]

    for root, _, files in os.walk(walk_dir):
        with open(outfile, 'a+') as out:
            out.write(f"{dim}\n")

            for filename in files:
                file_path = os.path.join(root, filename)

                with open(file_path, 'r') as infile:
                    matrix = [next(infile) for _ in range(int(dim))]

                for row in matrix:
                    out.write(row)
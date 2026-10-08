# C++ version

C++17 ports of the Needleman-Wunsch implementations live in `src/` as `*.cpp` and `*.h`.
The project is built and run as a C++-only codebase.

Build and run from the project root:

```sh
make                      # builds into cpp_bin/
./run_all_cpp.sh          # same pipeline as run_all.sh, using the C++ binaries
```

Individual programs:

```sh
./cpp_bin/NeedlemanWunschSequential [len1 len2]
./cpp_bin/NeedlemanWunschParallel 1000 1,2,4,8
./cpp_bin/NeedlemanWunschParallel -f datasets/data_100_A.txt datasets/data_100_B.txt 1,2,4,8
./cpp_bin/RealDatasetProcessor
./cpp_bin/CorrectnessTester
./cpp_bin/PerformanceBenchmark
./cpp_bin/PerformanceAnalysis
```

`performance.py` is kept as-is and reads `results/performance.csv`.

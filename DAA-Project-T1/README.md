# Needleman-Wunsch Sequence Alignment (C++ + Python plots)

Match +1, mismatch -1, gap -2. Sequential = full DP matrix + traceback. Parallel = wavefront (anti-diagonal) scoring with std::thread.

## Run
Put `ecoli.fasta` in the project root, then:
```
./run_all.sh
```
Or step by step: `make`, `./cpp_bin/RealDatasetProcessor`, `./cpp_bin/CorrectnessTester`, `./cpp_bin/PerformanceBenchmark`, `./cpp_bin/PerformanceAnalysis`, `python3 performance.py`.

Extra: `./cpp_bin/NeedlemanWunschSequential [n m]`, `./cpp_bin/NeedlemanWunschParallel 1000 1,2,4,8`, `./cpp_bin/NeedlemanWunschParallel -f datasets/data_100_A.txt datasets/data_100_B.txt 1,2,4,8`

Needs g++ (C++17), pthreads, Python 3 + pandas + matplotlib. Use a machine with 8+ cores for meaningful speedups.

# Needleman-Wunsch Sequence Alignment (C++)

This project implements the Needleman-Wunsch global alignment algorithm for DNA sequences using a C++ sequential baseline and a parallel wavefront implementation. The work is driven by two core requirements: correctness and performance.

The sequential version computes the full dynamic-programming table and produces the exact score. The parallel version keeps the same scoring logic but computes cells in a blocked wavefront pattern across anti-diagonals to expose parallelism while preserving dependencies.

## Project overview

- Sequential baseline: `src/NeedlemanWunschSequential.cpp`
- Parallel implementation: `src/NeedlemanWunschParallel.cpp`
- Correctness validation: `src/CorrectnessTester.cpp`
- Performance benchmarking: `src/PerformanceBenchmark.cpp`
- Dataset generation: `src/RealDatasetProcessor.cpp`
- Performance analysis: `src/PerformanceAnalysis.cpp`
- Graph generation: `performance.py`

## Scoring scheme

- Match: +1
- Mismatch: -1
- Gap: -2

The recurrence used is:

$$
F_{i,j} = \max(F_{i-1,j-1} + s(a_i, b_j), F_{i-1,j} - 2, F_{i,j-1} - 2)
$$

## Results summary

The project was evaluated on real DNA datasets of sizes 100, 500, 1000, 2000, 5000 and 10000 with thread counts 1, 2, 4 and 8.

| Size | Threads | Sequential (ms) | Parallel (ms) | Speedup | Efficiency |
|---:|---:|---:|---:|---:|---:|
| 100 | 1 | 0.007 | 0.007 | 0.920 | 0.920 |
| 100 | 2 | 0.007 | 0.023 | 0.290 | 0.145 |
| 100 | 4 | 0.007 | 0.028 | 0.243 | 0.061 |
| 100 | 8 | 0.007 | 0.049 | 0.138 | 0.017 |
| 500 | 1 | 0.185 | 0.185 | 1.000 | 1.000 |
| 500 | 2 | 0.185 | 0.241 | 0.769 | 0.385 |
| 500 | 4 | 0.185 | 0.241 | 0.767 | 0.192 |
| 500 | 8 | 0.185 | 0.331 | 0.560 | 0.070 |
| 1000 | 1 | 0.743 | 0.741 | 1.004 | 1.004 |
| 1000 | 2 | 0.743 | 0.780 | 0.954 | 0.477 |
| 1000 | 4 | 0.743 | 0.690 | 1.078 | 0.269 |
| 1000 | 8 | 0.743 | 0.903 | 0.823 | 0.103 |
| 2000 | 1 | 2.968 | 2.969 | 1.000 | 1.000 |
| 2000 | 2 | 2.968 | 2.715 | 1.093 | 0.547 |
| 2000 | 4 | 2.968 | 1.512 | 1.963 | 0.491 |
| 2000 | 8 | 2.968 | 1.617 | 1.835 | 0.229 |
| 5000 | 1 | 18.586 | 18.575 | 1.001 | 1.001 |
| 5000 | 2 | 18.586 | 16.029 | 1.160 | 0.580 |
| 5000 | 4 | 18.586 | 8.886 | 2.092 | 0.523 |
| 5000 | 8 | 18.586 | 7.757 | 2.396 | 0.299 |
| 10000 | 1 | 74.333 | 74.628 | 0.996 | 0.996 |
| 10000 | 2 | 74.333 | 71.507 | 1.040 | 0.520 |
| 10000 | 4 | 74.333 | 37.168 | 2.000 | 0.500 |
| 10000 | 8 | 74.333 | 26.958 | 2.757 | 0.345 |

Best measured speedup: 2.757x at size 10000 with 8 threads.

## Experimental setup

From `results/benchmark_environment.txt`:

- CPU: Apple M4
- Available cores: 10
- Compiler: Apple LLVM 17.0.0 (clang-1700.6.3.2)
- Flags: `-std=c++17 -O2 -Wall -pthread`
- Timing method: median of 7 runs after warm-up, using `std::chrono::steady_clock`
- Dataset sizes: 100, 500, 1000, 2000, 5000, 10000

## One-command reproduction

From the project root:

```sh
./run_all.sh
```

This command rebuilds the project and runs the full pipeline:

- dataset generation
- correctness validation
- performance benchmark
- summary generation
- graph creation

## Why parallel speedup is not perfect

The measured speedup is below linear for several reasons:

1. Thread startup and scheduling overhead is non-zero.
2. The wavefront still has synchronization points between tiles.
3. Short diagonals at the start/end of the matrix have very little work, so threads often wait.
4. Memory access patterns are less cache-friendly than the row-major sequential loop.
5. Amdahl's law limits the achievable speedup when part of the work remains serial.

This is visible in the real data: at size 100, 8 threads is slower than the sequential version, and at size 500 the 8-thread run is still only 0.560x speedup. For larger inputs, especially 5000 and 10000, the parallel implementation reaches 2.396x and 2.757x, respectively. This shows that the algorithm scales with problem size but remains limited by synchronization and memory overhead.

## Conclusion

The C++ project verifies correctness across all six dataset sizes and all tested thread counts, with the sequential and parallel scores matching exactly. The blocked wavefront design is substantially better than the naive barrier-per-diagonal approach, but the measured speedup remains below ideal linear scaling because synchronization, memory access, and serial bottlenecks still dominate for smaller inputs.

The project is therefore successful as a correctness-preserving and experimentally validated parallel implementation, with the best observed speedup reaching 2.757x on the 10000-size dataset with 8 threads.

# Needleman–Wunsch Sequence Alignment

A high-performance C++ implementation of the **Needleman–Wunsch global sequence alignment algorithm**, developed as a Design and Analysis of Algorithms (DAA) project. The project implements and compares three approaches: **Sequential, C++ `std::thread`, and OpenMP**, evaluating correctness, execution time, speedup, and scalability across different DNA sequence sizes.

---

## 1. Project Overview

Needleman–Wunsch is a dynamic programming algorithm used to find the optimal global alignment between two biological sequences.

For DNA sequences, the algorithm processes four nucleotide bases:

- `A` — Adenine
- `T` — Thymine
- `C` — Cytosine
- `G` — Guanine

The project investigates how parallel programming techniques can improve the performance of the traditional sequential algorithm using **blocked wavefront parallelism**.

### Main Objectives

- Implement the sequential Needleman–Wunsch algorithm.
- Develop a parallel implementation using `std::thread`.
- Implement parallel processing using OpenMP.
- Preserve the dependencies of the dynamic programming matrix.
- Verify that all implementations produce identical alignment scores.
- Benchmark different sequence sizes and thread configurations.
- Calculate speedup and analyze scalability.
- Compare the effectiveness of different parallelization approaches.

---

## 2. Algorithm

Needleman–Wunsch constructs a dynamic programming matrix `DP[i][j]`, where each cell represents the optimal alignment score between prefixes of the two sequences.

### Recurrence Relation

```text
DP[i][j] = max(
    DP[i-1][j-1] + score(sequence1[i-1], sequence2[j-1]),
    DP[i-1][j]   + gap_penalty,
    DP[i][j-1]   + gap_penalty
)
```

### Scoring Scheme

| Operation | Score |
|---|---:|
| Match | +1 |
| Mismatch | -1 |
| Gap | -2 |

### Initialization

```text
DP[0][0] = 0
DP[i][0] = i × gap_penalty
DP[0][j] = j × gap_penalty
```

The final cell `DP[n][m]` contains the optimal global alignment score.

---

## 3. Sequential Implementation

The sequential implementation calculates the DP matrix one cell at a time, processing rows and columns in order.

```text
for i = 1 to n
    for j = 1 to m
        calculate DP[i][j]
```

Each cell uses previously calculated neighboring cells. This implementation serves as the **baseline for correctness testing and performance comparisons**.

Implementation: `src/NeedlemanWunschSequential.cpp`

---

## 4. Parallel Implementations

### 4.1 C++ `std::thread`

The `std::thread` implementation uses **blocked/tiled wavefront parallelism** to calculate independent regions of the dynamic programming matrix concurrently.

The matrix is divided into smaller tiles. Tiles on the same anti-diagonal can execute simultaneously because their dependencies belong to previously completed anti-diagonals.

```text
          B00
       B10  B01
    B20  B11  B02
 B30  B21  B12  B03
```

The execution process is:

1. Divide the DP matrix into tiles.
2. Group tiles by anti-diagonal.
3. Process independent tiles using multiple threads.
4. Synchronize between dependent anti-diagonals.
5. Return the final alignment score.

This approach preserves the dependency structure while allowing multiple CPU threads to perform calculations concurrently.

Implementation: `src/NeedlemanWunschParallel.cpp`

### 4.2 OpenMP

The OpenMP implementation applies the same tiled-wavefront concept using OpenMP parallel loops and compiler directives.

OpenMP simplifies thread management by allowing the compiler and runtime to schedule independent work across available threads.

However, **the current benchmark shows that this implementation performs substantially worse than both the sequential and `std::thread` implementations on the tested system**.

This result indicates that the current OpenMP implementation or its runtime configuration requires further optimization. It does not establish that OpenMP is inherently slower than `std::thread`.

Implementation: `src/NeedlemanWunschOpenMP.cpp` *(adjust the filename if your repository uses a different name).*

---

## 5. Dataset

The project evaluates DNA sequence alignment using datasets generated from *E. coli* genomic data.

### Tested Sequence Sizes

```text
100 bp
500 bp
1,000 bp
2,000 bp
5,000 bp
10,000 bp
```

The experiments examine how performance changes as the computational workload increases.

Thread configurations tested:

```text
1, 2, 4, and 8 threads
```

---

## 6. Correctness Testing

All three implementations were tested against the same input datasets.

### Correctness Condition

```text
Sequential Score == std::thread Score == OpenMP Score
```

**Result: All correctness tests passed.**

The implementations produced identical final alignment scores across the tested sequence sizes.

This confirms that parallel execution preserves the final score for the tested inputs. It does not, by itself, establish that every implementation reconstructs an identical alignment string, since the current comparison evaluates scores.

---

## 7. Performance Benchmarking

Execution time was measured for the three implementations across multiple sequence sizes and thread configurations.

The benchmarking process uses:

- `std::chrono::steady_clock` for timing.
- Warm-up executions.
- Repeated measurements.
- Median execution time to reduce the influence of temporary system fluctuations.

The experiments were conducted on an Apple M4 system using Apple LLVM 17.0.0 and C++17.

The `std::thread` implementation was compiled using optimization flags such as:

```bash
-std=c++17 -O2 -Wall -pthread
```

The OpenMP implementation additionally requires appropriate OpenMP compiler and runtime support.

---

## 8. Performance Metrics

### Speedup

Speedup measures the improvement in execution time compared with the sequential baseline.

```text
Speedup = Sequential Execution Time / Parallel Execution Time
```

Interpretation:

- Speedup greater than 1: parallel execution is faster.
- Speedup equal to 1: both take the same time.
- Speedup below 1: parallel execution is slower.

### Parallel Efficiency

Efficiency measures how effectively the available threads contribute to performance.

```text
Efficiency = Speedup / Number of Threads
```

Efficiency is expressed as a fraction and can be multiplied by 100 to obtain a percentage.

For example, a speedup of 2.91× using eight threads gives:

```text
Efficiency = 2.91 / 8
           = 0.364
           ≈ 36.4%
```

---

## 9. Performance Results

### 9.1 Overall Comparison

| Metric | Sequential | `std::thread` | OpenMP |
|---|---:|---:|---:|
| Primary purpose | Baseline | Parallel execution | Parallel execution |
| Best reported speedup | 1.0× | **2.91×** | 0.58× |
| 10,000 bp execution time | 74.7 ms | **25.6 ms** | 358.9 ms |
| Large-input performance | Baseline | **Fastest** | Slowest |
| Current recommendation | Baseline comparisons | **Preferred implementation** | Requires optimization |

*Note: The reported 0.58× OpenMP speedup is its best reported result, at 500 bp with four threads. Speedup values below 1 indicate a slowdown relative to the sequential implementation.*

### 9.2 Small Dataset: 100 bp

| Implementation | Observed execution time |
|---|---:|
| Sequential | 0.006 ms |
| `std::thread` | 0.006–0.061 ms |
| OpenMP | 0.015–0.017 ms |

For small datasets, the actual computation is very short. Thread management, scheduling, and synchronization can cost more than the work being parallelized.

Consequently, parallel execution provides little benefit and may be significantly slower.

### 9.3 Medium Datasets: 500–1,000 bp

| Implementation | Observed behavior |
|---|---|
| Sequential | 0.185–0.743 ms |
| `std::thread` | Benefits depend on sequence size and thread count |
| OpenMP | Slower than sequential in the reported tests |

At these sizes, the workload may still be too small to compensate for parallelization overhead. The `std::thread` implementation begins to demonstrate benefits under more favorable configurations.

### 9.4 Large Datasets: 2,000–10,000 bp

The `std::thread` implementation performs best on the larger datasets, where more computation is available to distribute across threads.

For the 10,000 bp benchmark, the latest reported measurements are:

| Configuration | Execution time |
|---|---:|
| Sequential | 74.7 ms |
| `std::thread`, 8 threads | **25.6 ms** |
| OpenMP | 358.9 ms |

The `std::thread` implementation reduces execution time by approximately 65.7% compared with the sequential baseline.

The OpenMP implementation takes approximately 4.8 times as long as the sequential version for this dataset.

### 9.5 Best Reported Result

```text
Sequence size       : 10,000 bp
Implementation      : std::thread
Thread count        : 8
Sequential time     : 74.7 ms
Parallel time       : 25.6 ms
Speedup              : 2.91×
Parallel efficiency : 36.4%
```

These figures are based on the latest reported benchmark run. Earlier benchmark results should not be mixed with these measurements unless the runs are explicitly compared.

---

## 10. Performance Analysis

### Why Is `std::thread` Faster?

The `std::thread` implementation achieves the best measured performance because it distributes independent tiles across multiple threads while preserving the wavefront dependencies.

For large datasets, the computational workload is sufficiently substantial to offset much of the overhead associated with parallel execution.

### Why Is OpenMP Slower?

The current OpenMP implementation exhibits substantial overhead and poor performance on the tested system.

Possible factors include:

1. **Runtime overhead:** Parallel-region entry, scheduling, and synchronization may be expensive relative to the work assigned.
2. **Insufficient parallel work:** Small tiles or limited work per anti-diagonal can reduce useful parallel execution.
3. **Dependency synchronization:** Every wavefront must respect the completion of its predecessor dependencies.
4. **Compiler and runtime configuration:** OpenMP support and the selected runtime can affect performance on macOS.
5. **Memory access patterns:** DP matrix access and cache behavior can affect execution time.

These are potential explanations, not individually verified causes. Further profiling and controlled experiments are needed to identify the main bottleneck.

### Scalability

The `std::thread` implementation demonstrates the strongest scalability among the tested approaches for large inputs.

However, performance does not increase linearly with thread count. Eight threads do not automatically provide an eightfold speedup because of synchronization, data dependencies, scheduling overhead, and the finite amount of parallel work available in each wavefront.

---

## 11. Why Speedup Is Not Linear

The algorithm has inherent limitations that prevent perfect parallel scaling.

### Main Factors

1. **Data dependencies:** Each DP cell depends on neighboring cells that must be computed first.
2. **Synchronization:** Threads must wait for dependent wavefronts to finish.
3. **Uneven workload:** The number of independent tiles varies across anti-diagonals.
4. **Thread management:** Creating, scheduling, and coordinating threads introduces overhead.
5. **Memory access:** Cache misses and memory bandwidth can limit performance.
6. **Sequential work:** Initialization, coordination, and other serial operations cannot benefit fully from parallelization.

These factors explain why the observed 2.91× speedup is lower than the theoretical maximum of 8× for eight threads.

---

## 12. Experimental Environment

| Parameter | Configuration |
|---|---|
| CPU | Apple M4 |
| Available cores | 10 |
| Compiler | Apple LLVM 17.0.0 |
| Language standard | C++17 |
| Optimization | `-O2` |
| Parallel approaches | `std::thread`, OpenMP |
| Sequence sizes | 100–10,000 bp |
| Thread configurations | 1, 2, 4, 8 |

The results represent performance on this specific system and configuration. They should not be generalized to every CPU, compiler, or OpenMP runtime without further testing.

---

## 13. Project Structure

```text
DAA-Project-T1/
│
├── src/
│   ├── NeedlemanWunschSequential.cpp
│   ├── NeedlemanWunschParallel.cpp
│   ├── NeedlemanWunschOpenMP.cpp
│   ├── CorrectnessTester.cpp
│   ├── PerformanceBenchmark.cpp
│   ├── RealDatasetProcessor.cpp
│   └── PerformanceAnalysis.cpp
│
├── results/
│   ├── correctness.csv
│   ├── benchmark.csv
│   └── performance_summary.csv
│
├── performance.py
├── run_all.sh
├── Makefile
└── README.md
```

*Adjust filenames to match the actual repository structure.*

---

## 14. How to Run

### Clone the Repository

```bash
git clone https://github.com/pawarpranjali77/DAA-Project-T1.git
cd DAA-Project-T1
```

### Compile

```bash
make
```

### Run the Complete Pipeline

```bash
./run_all.sh
```

The pipeline is intended to perform the following steps:

```text
Dataset Preparation
        ↓
Correctness Testing
        ↓
Performance Benchmarking
        ↓
Performance Analysis
        ↓
Graph Generation
```

The actual commands and available targets depend on the repository's `Makefile` and `run_all.sh` implementation.

---

## 15. Output and Analysis

The project produces results that can be used to examine:

- Execution time for each implementation.
- Speedup relative to sequential execution.
- Parallel efficiency.
- The effect of sequence size.
- The effect of thread count.
- Scalability of the wavefront algorithm.
- Differences between `std::thread` and OpenMP.

Performance graphs can be generated using `performance.py`, provided that the script is configured for the current benchmark output format.

---

## 16. Limitations

The current project has the following limitations:

- Testing is limited to sequence sizes up to 10,000 bp.
- Small datasets may perform better sequentially.
- Parallel speedup is constrained by wavefront dependencies.
- Synchronization and scheduling introduce overhead.
- The full dynamic programming matrix requires substantial memory for larger sequences.
- The OpenMP implementation performs poorly under the tested configuration and requires further investigation.
- Performance results may vary with compiler settings, runtime configuration, and hardware.
- Score equality does not independently verify traceback or reconstructed alignment strings.

---

## 17. Future Improvements

Potential improvements include:

- Optimizing OpenMP scheduling and tile sizes.
- Investigating OpenMP compiler and runtime configuration.
- Reusing a persistent thread pool to reduce thread-management overhead.
- Implementing parallel traceback to reconstruct the complete alignment.
- Using rolling anti-diagonal buffers in score-only mode to reduce memory usage.
- Benchmarking larger datasets, such as 20,000 and 50,000 bp.
- Testing independent and mutated DNA sequences.
- Adding edge-case and known-answer tests.
- Profiling cache behavior, scheduling overhead, and synchronization costs.
- Comparing performance across additional hardware and compiler configurations.

---

## 18. Conclusion

This project demonstrates the application of dynamic programming and parallel programming to the Needleman–Wunsch global sequence alignment algorithm.

Three implementations were evaluated: Sequential, C++ `std::thread`, and OpenMP. All three passed the reported correctness tests and produced identical alignment scores across the tested datasets.

The latest benchmark results show that **the `std::thread` implementation is the best-performing approach in the current experimental environment**, achieving a reported speedup of 2.91× on 10,000 bp sequences using eight threads.

The OpenMP implementation performs significantly worse under the tested configuration, highlighting the importance of workload partitioning, synchronization, runtime configuration, and profiling when designing parallel algorithms.

Overall, the project illustrates that parallelization can substantially reduce execution time for computationally intensive workloads, but its effectiveness depends on the implementation and the available parallelism.

### Final Performance Summary

| Metric | Result |
|---|---:|
| Correctness | All three implementations passed |
| Largest tested sequence | 10,000 bp |
| Best implementation | `std::thread` |
| Best reported speedup | **2.91×** |
| Best reported parallel time | **25.6 ms** |
| Thread count for best result | 8 |
| Main conclusion | Parallelization benefits larger workloads, but overhead limits scalability |

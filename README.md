```markdown
# Needleman-Wunsch Sequence Alignment

A high-performance C++ implementation of the **Needleman-Wunsch global sequence alignment algorithm**, developed as a Design and Analysis of Algorithms (DAA) project.

The project implements both **sequential and parallel versions**, validates their correctness, and compares their performance across different DNA sequence sizes and thread configurations.

---

## 1. Project Overview

Needleman-Wunsch is a dynamic programming algorithm used to find the **optimal global alignment between two biological sequences**.

For DNA sequences, the algorithm compares:

- `A` — Adenine
- `T` — Thymine
- `C` — Cytosine
- `G` — Guanine

The project focuses on improving the computational performance of the traditional sequential algorithm using **parallel wavefront processing**.

### Main Objectives

- Implement the sequential Needleman-Wunsch algorithm.
- Implement a parallel version using C++ threads.
- Preserve the dependency structure of the dynamic programming matrix.
- Verify that parallel and sequential implementations produce identical scores.
- Benchmark different sequence sizes and thread counts.
- Calculate speedup and efficiency.
- Analyze the scalability and limitations of parallel execution.

---

## 2. Algorithm

Needleman-Wunsch uses a dynamic programming matrix `DP[i][j]`.

Each cell represents the best alignment score between prefixes of the two sequences.

The recurrence relation is:

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

Initialization:

```text
DP[0][0] = 0

DP[i][0] = i × gap_penalty

DP[0][j] = j × gap_penalty
```

---

## 3. Sequential Implementation

The sequential implementation calculates the DP matrix one cell at a time.

```text
for i = 1 to n
    for j = 1 to m
        calculate DP[i][j]
```

This provides the baseline execution time against which the parallel implementation is compared.

File:

```text
src/NeedlemanWunschSequential.cpp
```

---

## 4. Parallel Implementation

The DP matrix cannot simply be divided row-wise or column-wise because every cell depends on three previous cells:

```text
        DP[i-1][j]
             ↓
DP[i][j-1] → DP[i][j] ← DP[i-1][j-1]
```

Therefore, the project uses **blocked/tiled wavefront parallelism**.

The matrix is divided into blocks:

```text
        B00
     B10  B01
  B20  B11  B02
B30  B21  B12  B03
```

Blocks on the same anti-diagonal can be processed simultaneously because their required dependencies have already been completed.

This allows multiple threads to work concurrently while maintaining the correctness of the dynamic programming algorithm.

File:

```text
src/NeedlemanWunschParallel.cpp
```

---

## 5. Dataset

The project uses DNA sequence datasets generated from **E. coli genomic data**.

Tested sequence sizes:

```text
100
500
1000
2000
5000
10000
```

These different sizes help evaluate how the parallel implementation behaves as the computational workload increases.

---

## 6. Correctness Testing

The parallel implementation is tested against the sequential implementation.

For every dataset:

```text
Parallel Score == Sequential Score
```

The testing covers:

- Different sequence sizes
- Different thread counts
- Sequential vs parallel scores

Files:

```text
src/CorrectnessTester.cpp
```

The correctness test ensures that parallel execution does not change the final alignment score.

---

## 7. Performance Benchmarking

Performance is measured for:

```text
Threads:
1
2
4
8
```

For each configuration, execution time is recorded for different sequence sizes.

The benchmark uses:

- `std::chrono::steady_clock`
- Warm-up execution
- Multiple repeated runs
- Median execution time

Using the median reduces the effect of temporary system-level fluctuations.

File:

```text
src/PerformanceBenchmark.cpp
```

---

## 8. Performance Metrics

### Speedup

Speedup measures how much faster the parallel implementation is compared with the sequential implementation.

```text
Speedup = Sequential Time / Parallel Time
```

### Efficiency

Efficiency measures how effectively the available threads are being used.

```text
Efficiency = Speedup / Number of Threads
```

---

## 9. Performance Results

### Best Result

For a sequence size of **10,000**:

| Configuration | Time |
|---|---:|
| Sequential | 74.333 ms |
| Parallel – 8 Threads | 26.958 ms |
| Speedup | **2.757×** |
| Efficiency | **34.5%** |

### Speedup Summary

| Sequence Size | 2 Threads | 4 Threads | 8 Threads |
|---:|---:|---:|---:|
| 100 | 0.290× | 0.243× | 0.138× |
| 500 | 0.769× | 0.767× | 0.560× |
| 1000 | 0.954× | 1.078× | 0.823× |
| 2000 | 1.093× | 1.963× | 1.835× |
| 5000 | 1.160× | 2.092× | 2.396× |
| 10000 | 1.040× | 2.000× | **2.757×** |

---

## 10. Performance Analysis

The results show that parallelization becomes more beneficial as the sequence size increases.

For small inputs, parallel execution can be slower because the overhead of:

- Thread creation
- Synchronization
- Scheduling
- Memory access

can be larger than the actual computation.

For larger inputs, there is significantly more computation available for the threads, resulting in better speedup.

The highest measured speedup is:

```text
2.757×
```

for:

```text
Sequence Size = 10,000
Threads = 8
```

---

## 11. Why Speedup Is Not Linear

The speedup does not increase proportionally with the number of threads.

For example:

```text
1 thread  → baseline
2 threads → 1.040×
4 threads → 2.000×
8 threads → 2.757×
```

This is expected because the algorithm contains several sources of overhead.

### Main reasons

1. **Data Dependencies**

   DP cells cannot be calculated independently.

2. **Synchronization**

   Wavefront blocks must wait for dependent blocks.

3. **Uneven Work**

   The number of available blocks changes across different wavefronts.

4. **Thread Overhead**

   Managing multiple threads introduces additional cost.

5. **Memory Access**

   The DP matrix is large and memory access can limit performance.

6. **Sequential Portions**

   Some parts of the algorithm cannot be fully parallelized.

---

## 12. Experimental Environment

The experiments were performed on:

```text
CPU: Apple M4
Available Cores: 10
Compiler: Apple LLVM 17.0.0
Language: C++17
Optimization: -O2
Threading: std::thread
```

Compilation flags:

```text
-std=c++17 -O2 -Wall -pthread
```

Benchmark methodology:

```text
Warm-up runs
+
7 measured runs
+
Median execution time
```

---

## 13. Project Structure

```text
DAA-Project-T1/
│
├── src/
│   ├── NeedlemanWunschSequential.cpp
│   ├── NeedlemanWunschParallel.cpp
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

### Run Complete Pipeline

```bash
./run_all.sh
```

The pipeline performs:

```text
Dataset Generation
        ↓
Correctness Testing
        ↓
Performance Benchmarking
        ↓
Performance Analysis
        ↓
Graph Generation
```

---

## 15. Output

The project generates performance and correctness results that can be used to analyze:

- Execution time
- Speedup
- Parallel efficiency
- Effect of sequence size
- Effect of thread count
- Scalability of the parallel algorithm

Graphs are generated using:

```text
performance.py
```

---

## 16. Limitations

The current implementation has some limitations:

- Parallel execution uses `std::thread`.
- Speedup is limited by DP dependencies.
- Synchronization introduces overhead.
- Small datasets may perform better sequentially.
- Memory usage increases with the size of the DP matrix.
- Current testing is limited to sequence sizes up to 10,000.

---

## 17. Future Improvements

Possible improvements include:

- Parallel traceback for reconstructing the complete alignment.
- Rolling anti-diagonal buffers to reduce memory usage.
- Testing larger inputs such as 20,000 and 50,000.
- Reusing threads instead of repeatedly creating them.
- Testing independent or mutated DNA sequences.
- Stronger dataset validation.
- Additional edge-case tests.
- Known-answer test cases.
- OpenMP implementation.
- Further performance optimization.

---

## 18. Conclusion

This project demonstrates how the **Needleman-Wunsch dynamic programming algorithm** can be parallelized using a **blocked wavefront approach**.

The implementation maintains correctness while reducing execution time for larger sequences.

The best measured result was:

```text
Sequence Size : 10,000
Threads        : 8
Sequential     : 74.333 ms
Parallel       : 26.958 ms
Speedup        : 2.757×
```

The results show that parallelization is most effective for larger computational workloads, while synchronization and dependency constraints prevent perfectly linear speedup.
```

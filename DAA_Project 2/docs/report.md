# Needleman-Wunsch DNA Sequence Alignment

## Overview

This project implements global pairwise DNA sequence alignment with the
Needleman-Wunsch dynamic-programming algorithm. The sequential Java
implementation builds the full score matrix and reconstructs an alignment.
The parallel Java implementation calculates the alignment score using
wavefront (anti-diagonal) processing; it does not reconstruct aligned strings.

Both implementations use the same scoring scheme:

- Match: +1
- Mismatch: -1
- Gap: -2

## Algorithm and Implementation

For sequences `a` and `b`, each matrix cell takes the best of a match or
mismatch, a gap in either sequence, and the previous prefix scores:

$$
F_{i,j}=\max\left(F_{i-1,j-1}+s(a_i,b_j),\;F_{i-1,j}-2,\;F_{i,j-1}-2\right)
$$

The boundary conditions are `F[i,0] = -2i` and `F[0,j] = -2j`. The
sequential implementation takes $O(nm)$ time and stores an $O(nm)$ matrix.
The parallel implementation computes independent cells on each anti-diagonal,
using barriers between diagonals and three diagonal buffers. Its total work is
$O(nm)$ and its score-buffer storage is $O(n)$.

## Input Data and Correctness

`RealDatasetProcessor.java` reads `ecoli.fasta`, skips FASTA header lines, and
retains only A, T, C, and G. For each of the lengths 100, 500, 1000, 2000,
5000, and 10000, it writes two files under `datasets/`: sequence A begins at
base 0, and sequence B begins at base 500.

`CorrectnessTester.java` compares the sequential full-matrix score with the
parallel score for every dataset pair at 1, 2, 4, and 8 threads, and writes
`results/correctness.csv` (24 rows, one per size and thread count). All 24
comparisons match. Under the match +1 scheme the maximum possible score for a
pair of length n is n, and every stored score is below that.

Because B starts 500 bases after A, the sizes 100 and 500 compare
non-overlapping windows, so their scores are negative (-31 and -81). For
sizes of 1000 and above the windows overlap and the scores rise with size;
size 5000 gives 2500 and size 10000 gives 7500, which equals (n - 500) matches
minus 1000 gap penalties of 2.

## Performance Results

`PerformanceBenchmark.java` runs on the same `datasets/` pairs as the
correctness test and writes `results/performance.csv` with one row per size
and thread count (1, 2, 4, 8). Each row repeats the sequential and parallel
scores, and the sequential baseline is cross-checked against the full-matrix
score used in `correctness.csv`, so the scores in the two files are identical
for each size. Times are the median of 7 runs after a global JIT warm-up and 5
per-configuration warm-up runs. `results/benchmark_environment.txt` records
the core count, JVM, and OS for the run.

**Environment caveat.** The stored `performance.csv` was produced on a machine
with 1 available core (see `benchmark_environment.txt`). Thread counts above 1
are therefore oversubscribed there, and the speedup and efficiency figures for
2, 4, and 8 threads measure synchronization cost on one core, not scaling.
Re-run `./run_all.sh` on a machine with at least 8 cores before quoting
speedups. The explanation below does not depend on the core count.

## Why the parallel version is slower

Every recorded speedup, in both the earlier four-thread file and the new runs,
is below 1. The cause is the structure of the wavefront algorithm, supported by
the earlier four-thread timings.

1. **One barrier per anti-diagonal.** `parallelScore` calls
   `CyclicBarrier.await()` once for each of the n + m - 1 diagonals, so a
   10000 x 10000 run executes about 20,000 barriers per thread, and the cost of
   each barrier is paid regardless of how little work a diagonal holds.
2. **Parallel time follows the number of diagonals, not the number of
   cells.** In the earlier four-thread file, dividing parallel time by the
   number of diagonals (2n - 1) gives roughly 0.08 to 0.13 ms per diagonal for
   sizes 500 to 10000 (0.106, 0.081, 0.094, 0.109, 0.131). Sequential time
   grows with n x m, but the parallel time grew about in proportion to n. That
   pattern is what a barrier-dominated run looks like.
3. **Too little work per diagonal.** A diagonal holds at most min(n, m) cells,
   and each cell costs a few nanoseconds. Even at size 10000 a diagonal is
   about 10 microseconds of sequential work on average, which a barrier cost
   of tens of microseconds or more cancels out. The `MIN_PARALLEL_LEN = 512`
   rule makes this worse: any diagonal shorter than 512 cells is computed by
   thread 0 alone while the other threads only wait at the barrier. For
   sizes 100 and 500 every diagonal is below 512, so no parallel work happens
   at all and the entire gap is overhead.
4. **Thread setup is included in the timing.** Each `parallelScore` call
   creates and shuts down its own thread pool, which dominates at small sizes
   (size 100 took 57 ms against 6 ms sequentially in the earlier file).
5. **Part of the gap is not synchronization.** With 1 thread there are no
   barriers, yet the diagonal version is still 0.66 to 0.88 times as fast as
   the row-by-row baseline at sizes 1000 and above in the latest run. Walking
   the matrix by anti-diagonals is less cache-friendly than walking it by rows.

In the latest run on 1 core, parallel time also increases steadily from 1 to 8
threads at every size (for example 330 ms, 320 ms, 400 ms, 483 ms at size
10000), consistent with each barrier getting more expensive as threads are
added.

These measurements establish that the barrier-per-diagonal design dominates
the runtime; they do not isolate how much of the earlier four-thread slowdown
came from barriers versus pool creation versus memory access. A profile on a
multi-core machine would separate them.

Likely fixes, none applied here: process the matrix in blocks (tiles) so one
barrier covers many cells, keep one thread pool for all runs, replace
`CyclicBarrier` with a spin or `Phaser` based barrier, and use the row-based
loop as the 1-thread path.

## Limitations

The stored performance numbers come from a single-core machine and cannot
show scaling; see the environment caveat above. Timings on a shared or
virtualized machine are noisy, particularly below 10 ms. The older four-thread
file was not reproducible (no input or machine record), which is why it was
replaced.

`PerformanceAnalysis.java` reads `results/performance.csv` and writes
`results/performance_summary.csv`. `performance.py` reads the same performance
CSV and generates execution-time, speedup, and efficiency plots under
`results/graphs/`.

## Conclusion

The implementation provides sequential and wavefront score computation, a
dataset-processing path, score comparisons, and performance-analysis tools.
Sequential and parallel scores match for all 6 sizes at 1, 2, 4, and 8 threads,
and the correctness and performance files now use the same dataset pairs and
report the same scores. The parallel version is slower than the sequential one
in every recorded measurement. The barrier-per-diagonal design explains this,
and a meaningful speedup measurement requires re-running on a multi-core
machine or reducing synchronization (for example, blocked wavefronts).

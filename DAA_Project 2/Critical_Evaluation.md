# Critical Evaluation of LLM Output

The instruction sheet requires at least one case where an LLM suggestion was
modified, rejected, or found unsuitable. This document records four cases from
this project. Each one lists what was suggested, why it was considered, what was
changed, and how the final decision was verified.

Cases marked **(confirm)** need the owner to state which prompt and tool produced
the original suggestion. Cross-reference the `LLM_Usage_Log.md` row.

---

## Case 1: Parallelizing each row independently (rejected)

**What the LLM suggested.** Split the DP matrix by rows and let each thread compute
a different row at the same time. *(confirm: Fatima, prompt F1)*

**Why it was considered.** Rows are the natural loop in the sequential code, and a
row-parallel loop is the simplest thing to write with threads or OpenMP.

**Why it was rejected.** Each cell `D[i][j]` depends on `D[i-1][j-1]`, `D[i-1][j]`
and `D[i][j-1]`. The left neighbour is in the same row, and the other two are in the
previous row. Two rows running at once would read cells that are not yet computed,
which gives wrong scores or a race condition.

**What was done instead.** Wavefront (anti-diagonal) processing. All cells with the
same `i + j` depend only on earlier diagonals, so cells on one diagonal can run
concurrently with a synchronization point between diagonals.

**How it was verified.** Sequential and parallel scores match on all 6 dataset sizes
at 1, 2, 4 and 8 threads (`results/correctness.csv`).

---

## Case 2: One barrier per anti-diagonal (modified after measurement)

**What the LLM suggested.** Process the matrix one anti-diagonal at a time. Split
each diagonal across threads, and wait at a `CyclicBarrier` before starting the next
diagonal. *(confirm: Fatima, prompt F1)*

**Why it was considered.** It follows directly from the dependency analysis in
Case 1 and is correct.

**What went wrong.** The result was correct but slower than the sequential code. In
the first recorded benchmark (4 threads, size 10000): sequential 403.991 ms, parallel
2623.884 ms, speedup 0.154. Speedup was between 0.036 and 0.154 for every size. The
reasons found in the analysis:

1. A 10000 x 10000 run needs about 20,000 barriers, and each barrier costs about
   as much as the work on a short diagonal.
2. Diagonals near the matrix corners hold very little work, so threads mostly wait.
3. A thread pool was created on every call, which dominated the small sizes.

**What was changed.** The matrix was split into 128 x 128 tiles and the wavefront
runs over the tiles, so one synchronization step covers many cells. One thread pool is
reused, and the single-thread case uses the plain row-by-row loop.

**How it was verified.**

1. Correctness: `CorrectnessTester` still reports PASS for all sizes and thread counts.
2. Performance: re-benchmarked on an Apple M4 with 10 cores (median of 7 runs after
   warm-up). Size 10000: sequential 74.333 ms, parallel at 8 threads 26.958 ms,
   speedup 2.757. See `results/performance.csv` and
   `results/benchmark_environment.txt`.

Speedup is still below the thread count (efficiency 0.345 at 8 threads, size 10000).
The report discusses the remaining causes: tile-level synchronization, short
diagonals, cache behaviour, and Amdahl's law.

---

## Case 3: Stale correctness results (found and rejected)

**What was found.** `results/correctness.csv` listed a sequential score of 170 for
the size-100 pair, 860 for size 500, and so on, and the file marked every row as a match.

**Why it was suspicious.** With a match score of +1, the maximum possible score for
two sequences of length 100 is 100, so 170 cannot be correct. The values came from an
earlier scoring scheme (+2 / -1 / -1), not the current one (+1 / -1 / -2). The "match"
column was true only because both programs had used the same old scheme at that time.

**What was changed.** `CorrectnessTester` was re-run with the current scheme and the
file regenerated (now -31, -81, -129, -244, 2500, 7500).

**How it was verified.** A score above the theoretical maximum was the red flag. Then
the sequential and parallel scores were compared again, and for sizes 100, 500 and
1000 the score was recomputed with an independent Python implementation of the same
recurrence. All three methods agreed. A passing check is only trusted if the
inputs and scoring scheme it ran on are also known.

---

## Case 4: Benchmark results from the wrong run (discarded)

**What was found.** In the C++ repository, `results/performance.csv` and
`benchmark_environment.txt` looked complete, but the environment file said
`java=21` and `System.nanoTime()` and `cores_available=1`. The C++ benchmark records
the compiler and `std::chrono`, not a JVM. So the numbers came from the earlier Java
benchmark on a one-core machine, not from the C++ code the report described.

**Why it mattered.** On one core, runs with 2, 4 or 8 threads only measure
synchronization cost, so they say nothing about scaling. The report would also
have described C++ results that were not produced by C++.

**How it was checked.** The C++ project was rebuilt with `make` and the C++ benchmark
re-run. Its times differed from the committed file, which confirmed the mismatch.

**What was changed.** The benchmark was re-run on the Apple M4 (10 cores). The
environment file now records the CPU model, compiler flags, tile size and timing
method, so each result can be traced to the run that produced it.

---

## What these cases show

1. LLM-generated code that passes a correctness check can still be too slow to be
   useful (Case 2), so correctness and performance both have to be measured.
2. A passing result needs to be checked against its inputs and its configuration
   (Cases 3 and 4).
3. Each suggestion was accepted only after it was tested, compared with the
   sequential baseline, or recomputed independently.

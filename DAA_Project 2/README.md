# Needleman-Wunsch Sequence Alignment

This project contains sequential and parallel C++ implementations of
global DNA sequence alignment. Both use match `+1`, mismatch `-1`, and gap
`-2` scores. The sequential implementation builds the full dynamic-programming
matrix and reconstructs an alignment. The parallel implementation computes
the alignment score using wavefront (anti-diagonal) processing.

## Data and Tests

`src/RealDatasetProcessor.cpp` reads `ecoli.fasta`, ignores FASTA headers,
keeps only A, T, C, and G, and writes paired files in `datasets/` for lengths
100, 500, 1000, 2000, 5000, and 10000. For each length, sequence A starts at
base 0 and sequence B starts at base 500.

`src/CorrectnessTester.cpp` reads those pairs, compares sequential and
parallel scores at thread counts 1, 2, 4, and 8, and overwrites
`results/correctness.csv`. It compares scores; it does not validate dataset
characters or lengths.

## Requirements

- A C++17 compatible compiler (g++ or clang)
- pthread support
- Python 3, pandas, and matplotlib to generate graphs

Run commands from the project root.

## Compile

```sh
make
```

## Run

Generate the dataset files from `ecoli.fasta`:

```sh
./cpp_bin/RealDatasetProcessor
```

Run the score comparisons on those files:

```sh
./cpp_bin/CorrectnessTester
```

Run the sequential implementation's built-in examples, or benchmark random
sequences with the two lengths supplied as arguments:

```sh
./cpp_bin/NeedlemanWunschSequential
./cpp_bin/NeedlemanWunschSequential 1000 1000
```

Run the parallel benchmark on random sequences. The optional second argument
is a comma-separated thread list; if omitted, it uses 1, 2, 4, and 8 threads.
The `-f` form reads two plain sequence files (not FASTA files):

```sh
./cpp_bin/NeedlemanWunschParallel 1000 1,2,4,8
./cpp_bin/NeedlemanWunschParallel -f datasets/data_100_A.txt datasets/data_100_B.txt 1,2,4,8
```

## Performance Results

`PerformanceBenchmark` runs sequential and parallel scoring on the same
`datasets/` pairs as `CorrectnessTester`, at 1, 2, 4, and 8 threads, and writes
`results/performance.csv` and `results/benchmark_environment.txt`:

```sh
./cpp_bin/PerformanceBenchmark
```

`./run_all.sh` regenerates every result in order. Run it on a machine with at
least 8 cores for meaningful speedups.

`NeedlemanWunschParallel` can also print CSV-formatted measurements for a
single input to standard output. `PerformanceAnalysis` reads
`results/performance.csv`, prints its rows and best speedup, and overwrites
`results/performance_summary.csv`:

```sh
./cpp_bin/PerformanceAnalysis
```

`performance.py` also reads `results/performance.csv` and writes
`execution_time.png`, `speedup.png`, and `efficiency.png` to
`results/graphs/`:

```sh
python3 performance.py
```

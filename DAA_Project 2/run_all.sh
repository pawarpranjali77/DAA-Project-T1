#!/bin/sh
# Regenerates every result from the SAME dataset files, in order.
# Run from the project root on the machine you want to report numbers for
# (use a machine with >= 8 cores so the 8-thread rows are meaningful).
set -e
mkdir -p results
make
./cpp_bin/RealDatasetProcessor      # datasets/ from ecoli.fasta
./cpp_bin/CorrectnessTester         # results/correctness.csv
./cpp_bin/PerformanceBenchmark      # results/performance.csv + benchmark_environment.txt
./cpp_bin/PerformanceAnalysis       # results/performance_summary.csv
python3 performance.py               # results/graphs/*.png
cat results/benchmark_environment.txt

#!/bin/sh
# C++ equivalent of run_all.sh. Run from the project root.
set -e
mkdir -p results
make
./cpp_bin/RealDatasetProcessor      # datasets/ from ecoli.fasta
./cpp_bin/CorrectnessTester         # results/correctness.csv
./cpp_bin/PerformanceBenchmark      # results/performance.csv + benchmark_environment.txt
./cpp_bin/PerformanceAnalysis       # results/performance_summary.csv
python performance.py               # results/graphs/*.png
cat results/benchmark_environment.txt

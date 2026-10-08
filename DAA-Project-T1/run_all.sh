#!/bin/bash
set -e
make
./cpp_bin/RealDatasetProcessor
./cpp_bin/CorrectnessTester
./cpp_bin/PerformanceBenchmark
./cpp_bin/PerformanceAnalysis
python3 performance.py

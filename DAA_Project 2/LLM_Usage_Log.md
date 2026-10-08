# LLM Usage Log

## Team members

### Mahi
- Role: dataset generation and sequence validation
- Real project use: reviewed the requirements for sequence extraction from `ecoli.fasta` and the dataset generation logic in `src/RealDatasetProcessor.cpp`.
- Notes: confirmed the data sizes and validation assumptions for 100, 500, 1000, 2000, 5000 and 10000.

### Pranjali
- Role: project setup, benchmarking workflow, and final documentation
- Real project use: checked the root build flow, benchmark environment output, reproducibility requirements, and validation of the project pipeline via `make` and `./run_all.sh`.
- Notes: ensured the repo runs from the root and the generated results are consistent with the actual benchmark output.

### Fatima
- Role: parallel implementation design
- Real project use: reviewed the wavefront dependency structure and the blocked-tile approach used in `src/NeedlemanWunschParallel.cpp`.
- Notes: focused on preserving sequential correctness while reducing synchronization overhead between tiles.

### Maitreyi
- Role: performance analysis and graph generation
- Real project use: validated the CSV structure and ensured `performance.py` produced the required graphs from `results/performance.csv`.
- Notes: verified the plots for execution time, speedup and efficiency.

## Rejected approach

### Row-wise independent parallelization
- Status: rejected
- Reason: the Needleman-Wunsch recurrence depends on neighboring cells and previous-row values, so cells cannot be computed independently without violating DP dependencies.
- Outcome: a wavefront/anti-diagonal strategy was selected instead to preserve correctness while exposing parallelism.

## Project workflow notes

- Build: `make`
- Full reproduction: `./run_all.sh`
- Compiler: `g++` with C++17
- Parallelism: `std::thread`
- Timing: `std::chrono::steady_clock`
- Result files: `results/correctness.csv`, `results/performance.csv`, `results/benchmark_environment.txt`, `results/graphs/*.png`

# LLM Ustbin/
cpp_bin/ Bcppd *.class`m
# mac Fu.DS_Store
**/.DS_St`.**/.DS_S.s.vscode/
ThivThumbs.t 
# LocalThepush.sh
do_push.py
EOypdo_pusreEOF
cat > `cava_version/` as a historical first version. It is not part of the active C++ build or validation path.

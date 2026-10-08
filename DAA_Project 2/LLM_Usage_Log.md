# LLM Usage Log

**Project:** Needleman-Wunsch Parallelization (Q3, Biological Sequence Alignment)
**Team:** Mahi, Pranjali, Fathima, Maitreyi
**Tools used:** Claude (claude.ai) <!-- TEAM: add any other tool you used, e.g. ChatGPT, Copilot -->

Prompts are recorded exactly as typed, including spelling mistakes. Rows marked
`TODO` must be filled by the owner with a prompt they **actually used**. Delete a
row if you did not use an LLM for that activity. Do not write prompts after the fact.

Verification summary used in several rows below: sequential and parallel scores
were compared for all 6 dataset sizes at 1, 2, 4 and 8 threads
(`results/correctness.csv`: -31, -81, -129, -244, 2500, 7500). For sizes 100, 500
and 1000 the score was also recomputed with an independent Python implementation.

---

## Mahi: datasets and documentation

| Sr. No. | Purpose | Prompt Used | How the Response Was Used | Modification / Verification |
|---|---|---|---|---|
| M1 | Dataset generation | "Generate a Java program to create DNA sequences containing A, T, C and G for dataset sizes 100, 500, 1000, 2000, 5000 and 10000." | Used to create the first synthetic dataset generator. | Replaced later by `RealDatasetProcessor`, which reads real E. coli bases from `ecoli.fasta` and writes the paired files `data_<size>_A.txt` / `_B.txt`. Verified by `CorrectnessTester` reading the same files. |
| M2 | Dataset validation | "Generate Java code to validate DNA datasets by checking sequence length and valid DNA characters." | Considered for a standalone validator. | No standalone `DatasetValidator` was added. `CorrectnessTester` compares scores only; it does not check lengths or characters. TODO (Mahi): state how lengths and the A/T/C/G character set were actually verified. |
| M3 | README / documentation | "Can you help me write a README for this project? Include what the project does, how to generate and use the datasets, how to run the Java programs, the different implementations, and how to run the tests. Keep it simple and clear." | Used as a starting point for structuring the project documentation and explaining how to run the different parts of the project. | README content was checked against the actual project files, class names, commands, dataset files, and results. Any instructions that did not match the final implementation were corrected manually. |

## Pranjali: sequential implementation, review and planning

| Sr. No. | Purpose | Prompt Used | How the Response Was Used | Modification / Verification |
|---|---|---|---|---|
| P1 | Algorithm understanding: sequential Needleman-Wunsch (recurrence, traceback) | "Can you explain the Needleman-Wunsch algorithm for sequence alignment in simple terms? I need to understand the scoring matrix, recurrence relation and traceback step, and then implement the sequential version in C++." | Used the explanation to understand the DP matrix, scoring rules and traceback process before implementing the sequential version. | Checked the recurrence and traceback logic against the course material and tested the implementation on small sequences where the alignment could be calculated manually. |
| P2 | Project status review (first repo) | "https://github.com/Mahi-sheth/DAA_Project this is the github linka nd this is the project please help us tell 3 things if the entire project is completed, how do we present it to sir in github only, and what should we say" | Found that `performance.csv` and `report.md` were empty, `correctness.csv` held stale scores, the README mentioned a class that did not exist, and the 4-thread timings showed the parallel code slower than sequential. Used the list to assign fixes to each member. | Findings were checked by re-cloning the repo, compiling, and re-running the sequential and parallel code on all datasets. The stale scores were confirmed (see `docs/Critical_Evaluation.md`, case 3). |
| P3 | Review after switching to C++ | "https://github.com/pawarpranjali77/DAA-Project-T1 check this now we have tried making it in cpp but some parts in java because my other class mates have made it in cpp only" | Found that the committed `performance.csv` came from the earlier Java run on a 1-core machine, that the report still said Java, and that binaries and helper scripts were committed. | Rebuilt with `make`, re-ran `CorrectnessTester` (all PASS) and the C++ benchmark. Timings differed from the committed file, which confirmed the provenance problem (case 4). Results were then regenerated on the Apple M4. |
| P4 | Improvement planning | "Can you give me 3 simple prompts to improve the whole project and make it an A-tier project? I want the prompts to cover the code/repo, performance and benchmarking, and the report/README/viva preparation." | Received 3 suggested prompts: (1) clean and flatten the repo, (2) fix the slow parallel code and re-benchmark, (3) finish the report, README and viva notes. Used them to plan the remaining improvements. | Changes were checked by reviewing the final repository structure, running `CorrectnessTester` (all PASS), re-running the benchmark, and checking that the report and README matched the final implementation. |
| P5 | Requirement compliance check | "Now tell me if the project is according to these requirements. We are going to include a few things in the report as well. Based on the requirements, how much would you rate the project and what should we improve? The topic is Q3 Biological Sequence Alignment." | Identified the gaps against the instruction sheet: no OpenMP/MPI/CUDA, missing LLM log, and missing individual contribution statements. | Checked each requirement against the instruction sheet and updated the project/report where applicable. Remaining gaps were explicitly documented rather than claiming they were completed. |

## Fathima: parallel implementation

| Sr. No. | Purpose | Prompt Used | How the Response Was Used | Modification / Verification |
|---|---|---|---|---|
| F1 | Parallelization: wavefront / anti-diagonal approach | I’m trying to parallelize matrix multiplication using a wavefront or anti-diagonal approach. Can you explain how the anti-diagonals work, what dependencies I need to consider, and how I can implement it? | Used the explanation to understand the wavefront approach and implement the parallel version. | Tested with different matrix sizes and thread counts and checked the output against the sequential version. |
| F2 | Optimization: reducing barrier cost (tiled / blocked wavefront) | My wavefront implementation has a lot of barrier synchronization and the speedup is not very good. How can I reduce the barrier overhead? Would tiling or blocked wavefronts help? | Used the suggested tiled/blocked wavefront approach to reduce synchronization overhead. | Tile size 128; compared experimentally. Size 10000, 8 threads: 2.757x speedup. |
| F3 | Debugging | My parallel matrix multiplication is giving wrong results. Can you help me find the issue? Please check the wavefront dependencies, synchronization, thread handling, and indexing. | Used the suggestions to find and fix issues in the implementation. | Compared the parallel results with the sequential version for different matrix sizes and thread counts. |

## Maitreyi: performance analysis

| Sr. No. | Purpose | Prompt Used | How the Response Was Used | Modification / Verification |
|---|---|---|---|---|
| Y1 | Benchmark design (warm-up, median of runs, thread counts) | I need to benchmark my parallel matrix multiplication program. How should I do the benchmarking properly? I want to compare 1, 2, 4 and 8 threads for different matrix sizes. Should I do a warm-up run and multiple runs? Also, should I use mean or median for the final timing? | Used to structure the benchmark harness: a warm-up run discarded before timing, the median of repeated runs reported instead of the mean, and thread counts 1, 2, 4 and 8 across all 6 dataset sizes. | Timings were taken on the Apple M4 (not the earlier 1-core Java run, see Critical Evaluation case 4). Scores were checked against `results/correctness.csv` before any timing was accepted. Results are in `results/performance.csv`. |
| Y2 | Graph generation (`performance.py`) | I have a performance.csv file with my matrix multiplication results. Can you make a Python script called performance.py that reads the CSV and plots execution time and speedup for different thread counts and matrix sizes? Make the graphs clear with proper labels and units. | Used to produce the first version of `performance.py`, which reads `performance.csv` and plots time and speedup against thread count for each dataset size. | Plotted values were compared against the rows in `performance.csv`. Axis labels and units were corrected by hand. Example check: the size 10000, 8-thread point matches the recorded 2.757x speedup. |
| Y3 | Analysis of bottlenecks / why speedup is not linear | My speedup is not increasing linearly when I increase the number of threads. What could be the reasons for this in parallel matrix multiplication? Could barrier overhead, memory bandwidth, matrix corners or tile size be causing it? Also tell me how I can test which one is actually affecting my results. | Used to list candidate causes: barrier cost per tile diagonal, limited parallelism near the matrix corners, memory bandwidth, and tile-size trade-offs. | Not accepted as-is. Barrier and tile-size explanations were tested by comparing tile-size runs (tile size 128 was chosen experimentally). Claims not backed by a measurement were removed from the report. |
---

## Critical evaluation of LLM output

At least one LLM suggestion was rejected or modified. The full write-up (what was
suggested, why it was considered, what was changed, how it was verified) is in
[`docs/Critical_Evaluation.md`](docs/Critical_Evaluation.md). Summary:

1. Row-wise parallelization: rejected (cell dependencies).
2. One barrier per anti-diagonal: replaced by a tiled wavefront after measuring a slowdown.
3. Stale `correctness.csv`: rejected after a score that was impossible for the scoring scheme.
4. Benchmark results from the wrong run (Java, 1 core): discarded and re-measured.

# Additional LLM Usage Log Entries

**Project:** Needleman–Wunsch Sequence Alignment  
**Team:** Mahi, Pranjali, Fathima, Maitreyi  
**Tools used:** Claude (claude.ai)

> Note: The following are suggested entries. Retain only prompts that accurately reflect the team's actual LLM usage.

---

## Mahi: Datasets and Documentation

| Sr. No. | Purpose | Prompt Used | How the Response Was Used | Modification / Verification |
|---|---|---|---|---|
| M4 | Real DNA dataset processing | "Help me implement a C++ program that reads DNA sequences from an E. coli FASTA file and generates paired datasets of sizes 100, 500, 1000, 2000, 5000 and 10000 bp." | Used to plan the generation of DNA sequence datasets from genomic data. | Verified sequence lengths, generated files and compatibility with the existing correctness tester. |
| M5 | Dataset validation | "Design validation checks for our Needleman–Wunsch DNA datasets to verify sequence lengths, valid nucleotide bases, missing files and consistent input loading across all three implementations." | Used to identify dataset validation requirements. | Checked validation behavior against the generated datasets and applicable error cases. |
| M6 | Repository organization | "Suggest a clean directory structure for our C++ Needleman–Wunsch project containing source files, datasets, correctness results, benchmark CSVs, performance graphs and documentation." | Used to review the organization of project files and outputs. | Compared the suggestions with the actual repository and existing build scripts. |
| M7 | Reproducible execution instructions | "Help document how to compile and run our Needleman–Wunsch project using the Makefile and run_all.sh script, including dataset preparation, correctness testing, benchmarking and graph generation." | Used to improve the project's setup and execution documentation. | Verified commands and filenames against the repository. |
| M8 | Technical documentation review | "Review our Needleman–Wunsch README for accuracy, including the dynamic programming recurrence, scoring scheme, tiled wavefront dependencies, parallel implementations, dataset sizes and performance metrics." | Used to identify documentation gaps and technical inconsistencies. | Cross-checked the documentation against the implementation and available benchmark results. |

---

## Pranjali: Sequential Implementation, Integration and Review

| Sr. No. | Purpose | Prompt Used | How the Response Was Used | Modification / Verification |
|---|---|---|---|---|
| P6 | Sequential implementation review | "Review our sequential C++ Needleman–Wunsch implementation. Verify DP matrix initialization, match score +1, mismatch score -1, gap penalty -2, recurrence relation and boundary conditions." | Used to review the sequential implementation serving as the performance baseline. | Compiled the implementation and checked its results against manually calculated examples and parallel outputs. |
| P7 | Traceback and alignment reconstruction | "Explain how traceback reconstructs the optimal global alignment from the Needleman–Wunsch DP matrix. Help implement traceback in C++ while correctly handling diagonal, upward and leftward moves." | Used to understand alignment reconstruction and identify requirements beyond returning the final score. | Checked alignment validity and score consistency using applicable tests. |
| P8 | Correctness tester | "Help design a C++ correctness tester that runs the sequential, std::thread and OpenMP Needleman–Wunsch implementations on identical DNA datasets and compares their final alignment scores, reporting PASS or FAIL." | Used to plan automated correctness verification across the three implementations. | Verified that the tester used identical input files and compared actual computed scores. |
| P9 | Repository integration review | "Review our Needleman–Wunsch repository for integration issues across the sequential, std::thread and OpenMP implementations, correctness tester, benchmark program and Makefile. Identify inconsistent interfaces, stale results and unsupported documentation claims." | Used to identify integration issues before final project evaluation. | Rebuilt the project and checked source files, execution commands and generated results. |
| P10 | Memory optimization | "Explain how rolling anti-diagonal buffers can reduce memory usage in score-only Needleman–Wunsch computation. Describe the dependencies that must be preserved and why reconstructing the full alignment requires additional information or another traceback strategy." | Used to evaluate memory optimization opportunities for score-only computation. | Reviewed the memory requirements and distinguished score-only computation from full alignment reconstruction. |

---

## Fathima: Parallel Implementation

| Sr. No. | Purpose | Prompt Used | How the Response Was Used | Modification / Verification |
|---|---|---|---|---|
| F3 | Tiled wavefront parallelization | "Help design a tiled-wavefront Needleman–Wunsch implementation in C++. Divide the DP matrix into tiles, process tiles by anti-diagonal and preserve top, left and top-left dependencies between tiles." | Used to understand and develop tile-level parallelism while preserving the algorithm's dependencies. | Compared the dependency logic with the sequential algorithm and tested final scores. |
| F4 | std::thread synchronization | "Review synchronization in our tiled-wavefront Needleman–Wunsch implementation using C++ std::thread. Explain how workers can process independent tiles and coordinate completion between anti-diagonals without data races or premature reads." | Used to evaluate thread coordination and safe execution of independent tiles. | Rebuilt and tested the implementation across supported thread counts against the sequential baseline. |
| F5 | OpenMP implementation | "Help implement tiled-wavefront Needleman–Wunsch using OpenMP parallel loops. Preserve dependencies between tile anti-diagonals and explain where synchronization is required to ensure correct DP values." | Used to review the OpenMP parallelization strategy. | Compiled with the available OpenMP toolchain and compared scores against the other implementations. |
| F6 | Thread-count scalability | "Explain how tile size, available independent tiles, scheduling and synchronization affect Needleman–Wunsch speedup when using 1, 2, 4 and 8 threads across different DNA sequence sizes." | Used to identify factors that could explain performance differences across thread counts. | Compared potential explanations with actual benchmark measurements before drawing conclusions. |
| F7 | Parallel edge-case testing | "Identify edge cases for our parallel Needleman–Wunsch implementations, including empty sequences, single-base sequences, identical sequences, mismatched sequences and unequal sequence lengths. Explain the expected behavior under our scoring scheme." | Used to plan additional correctness tests for boundary conditions. | Compared expected results with the sequential implementation and retained only tests that were executed. |

---

## Maitreyi: Performance Analysis and Benchmarking

| Sr. No. | Purpose | Prompt Used | How the Response Was Used | Modification / Verification |
|---|---|---|---|---|
| Y3 | Benchmark CSV validation | "Suggest a consistent CSV format for our Needleman–Wunsch benchmarks containing sequence size, thread count, execution time, speedup, parallel efficiency and correctness status." | Used to organize benchmark results for performance comparison. | Matched the proposed format with the benchmark program and checked calculations against recorded measurements. |
| Y4 | Performance graph generation | "Help create Python graphs from our Needleman–Wunsch benchmark CSV files comparing sequential, std::thread and OpenMP execution times across DNA sequence sizes and thread counts." | Used to plan performance visualizations for the project report. | Checked graph values against the source CSV files and excluded unsupported or outdated measurements. |
| Y5 | Scalability analysis | "Analyze our Needleman–Wunsch benchmark results for 1, 2, 4 and 8 threads. Explain speedup and parallel efficiency, identify configurations that improve performance and distinguish measured results from possible explanations for slowdowns." | Used to structure the scalability analysis and compare parallel implementations with the sequential baseline. | Recalculated speedup and efficiency using the relevant execution times and checked conclusions against the benchmark data. |
| Y6 | Fair benchmark methodology | "Suggest a fair methodology for comparing sequential, std::thread and OpenMP Needleman–Wunsch implementations on an Apple M4 system. Consider compiler optimization, identical datasets, warm-up runs, repeated measurements, thread counts and median execution time." | Used to review the experimental setup and improve benchmark consistency. | Compared the methodology with the actual build configuration, benchmark code and experimental environment. |
| Y7 | Performance report interpretation | "Help write the performance analysis for our Needleman–Wunsch DAA project. Explain observed differences between sequential, std::thread and OpenMP execution times, the effect of increasing sequence length and limitations of parallel speedup without assuming OpenMP is inherently slower." | Used to structure the interpretation of execution time, speedup and scalability results. | Retained numerical claims only when supported by benchmark data and treated unverified bottleneck explanations as hypotheses. |

---


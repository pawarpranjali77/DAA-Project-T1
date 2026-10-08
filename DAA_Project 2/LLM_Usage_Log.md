# LLM Usage Log

**Project:** Needleman-Wunsch Parallelization (Q3, Biological Sequence Alignment)  
**Team:** Mahi, Pranjali, Fathima, Maitreyi  
**Tools used:** Claude (claude.ai)

Prompts are included only for activities where an LLM was used. The prompts below should
be replaced with the exact original wording if the original prompt is available. No prompt
should be presented as an exact historical prompt unless it was actually typed.

---

## Mahi: datasets and documentation

| Sr. No. | Purpose | Prompt Used | How the Response Was Used | Modification / Verification |
|---|---|---|---|---|
| M1 | Dataset generation | "Generate a Java program to create DNA sequences containing A, T, C and G for dataset sizes 100, 500, 1000, 2000, 5000 and 10000." | Used to create the first synthetic dataset generator. | Replaced later by `RealDatasetProcessor`, which reads real E. coli bases from `ecoli.fasta` and writes the paired files `data_<size>_A.txt` / `_B.txt`. Verified by `CorrectnessTester` reading the same files. |
| M2 | Dataset and documentation review | "Can you check if the way we are generating and storing our DNA datasets is suitable for the Needleman-Wunsch project? Also tell me what should be mentioned in the README about the datasets." | Used to review the dataset organization and decide what dataset information should be documented. | Final README and dataset files were checked against the actual repository structure. |
| M3 | README / documentation | "Can you help me write a README for this project? Include what the project does, how to generate and use the datasets, how to run the Java programs, the different implementations, and how to run the tests. Keep it simple and clear." | Used as a starting point for structuring the project documentation. | README content was checked against the actual project files, commands, datasets and results. |

---

## Pranjali: sequential implementation, review and planning

| Sr. No. | Purpose | Prompt Used | How the Response Was Used | Modification / Verification |
|---|---|---|---|---|
| P1 | Algorithm understanding: sequential Needleman-Wunsch | "Can you explain the Needleman-Wunsch algorithm for sequence alignment in simple terms? I need to understand the scoring matrix, recurrence relation and traceback step, and then implement the sequential version in C++." | Used to understand the DP matrix, scoring rules and traceback process before implementing the sequential version. | Checked the recurrence and traceback logic against the course material and tested the implementation on small sequences. |
| P2 | Project status review (first repo) | "https://github.com/Mahi-sheth/DAA_Project this is the github linka nd this is the project please help us tell 3 things if the entire project is completed, how do we present it to sir in github only, and what should we say" | Found that `performance.csv` and `report.md` were empty, `correctness.csv` held stale scores, the README mentioned a class that did not exist, and the 4-thread timings showed the parallel code slower than sequential. Used the findings to assign fixes. | Findings were checked by re-cloning the repo, compiling, and re-running the sequential and parallel code. |
| P3 | Review after switching to C++ | "https://github.com/pawarpranjali77/DAA-Project-T1 check this now we have tried making it in cpp but some parts in java because my other class mates have made it in cpp only" | Reviewed the updated repository and identified issues with benchmark provenance, report language, and committed binaries/helper files. | Rebuilt with `make`, re-ran correctness and the C++ benchmark, and regenerated results on the Apple M4. |
| P4 | Improvement planning | "can you give prompts to improve the whole project and make A tier project give 5 lines ke 3 prompts to solve all the problems" | Used the response to identify possible areas for improvement in the repository, performance and documentation. | Suggested improvements were reviewed against the actual repository before being implemented. |
| P5 | Requirement compliance check | "now tell if the project is according to this requiremnet some common requirements but like we are goibg to inlcude few things in the report as well but based on htis how much would you rate the project give feedback as well Q3. Biological Sequence Alignment" | Used to identify gaps against the course requirements, including the parallel programming and documentation requirements. | Checked the identified gaps against the course instruction sheet and documented remaining gaps. |

---

## Fathima: parallel implementation

| Sr. No. | Purpose | Prompt Used | How the Response Was Used | Modification / Verification |
|---|---|---|---|---|
| F1 | Wavefront / anti-diagonal parallelization | "I’m implementing parallel Needleman-Wunsch. Can you explain how the anti-diagonal or wavefront approach works and how I can process independent parts in parallel without breaking the dependencies between DP cells?" | Used to understand the wavefront dependency pattern and guide the parallel implementation. | Tested the parallel results against the sequential implementation for correctness. |
| F2 | Synchronization and performance | "My parallel Needleman-Wunsch implementation uses synchronization between wavefronts and the speedup is not very good. What can I do to reduce synchronization overhead while still keeping the dependencies correct?" | Used to identify synchronization overhead and consider a blocked/tiled wavefront approach. | The implementation was tested using the existing correctness and performance tests. |

---

## Maitreyi: performance analysis

| Sr. No. | Purpose | Prompt Used | How the Response Was Used | Modification / Verification |
|---|---|---|---|---|
| Y1 | Benchmark design | "How should I benchmark a sequential and parallel Needleman-Wunsch program properly? I want to compare different dataset sizes and thread counts. What should I measure and how should I make the comparison fair?" | Used to plan the benchmark setup and compare execution times across dataset sizes and thread counts. | Used the same datasets and thread counts for the sequential and parallel implementations and checked the recorded results. |
| Y2 | Speedup and performance graphs | "I have execution time results for sequential and parallel Needleman-Wunsch. How should I calculate speedup and what graphs would be useful to show how performance changes with the number of threads?" | Used to plan the speedup calculation and performance graphs for the report. | Graphs were checked against the actual benchmark CSV values before being used in the report. |

---

## Critical evaluation of LLM output

At least one LLM suggestion was rejected or modified. The full write-up
(what was suggested, why it was considered, what was changed, and how it
was verified) is in
[`docs/Critical_Evaluation.md`](docs/Critical_Evaluation.md).

Summary:

1. Row-wise parallelization was rejected because Needleman-Wunsch has dependencies between DP cells.
2. The initial synchronization approach was modified in favour of the tiled wavefront approach.
3. Stale correctness results were rejected and correctness was re-tested.
4. Benchmark results from the earlier Java/1-core run were discarded and regenerated on the Apple M4.
5. Matrix-multiplication prompts that were unrelated to this Needleman-Wunsch project were removed from the log.
6. Performance claims were kept only when supported by actual benchmark results.

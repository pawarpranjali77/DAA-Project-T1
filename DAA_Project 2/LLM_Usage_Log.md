# LLM Usage Log

**Project:** Needleman-Wunsch Parallelization (Q3, Biological Sequence Alignment)
**Team:** Mahi, Pranjali, Fatima, Maitreyi
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
| M3 | README / documentation | TODO: paste actual prompt | TODO | TODO |

## Pranjali: sequential implementation, review and planning

| Sr. No. | Purpose | Prompt Used | How the Response Was Used | Modification / Verification |
|---|---|---|---|---|
| P1 | Algorithm understanding: sequential Needleman-Wunsch (recurrence, traceback) | TODO: paste actual prompt | TODO | TODO: e.g. checked against textbook / hand-computed small case |
| P2 | Project status review (first repo) | "https://github.com/Mahi-sheth/DAA_Project this is the github linka nd this is the project please help us tell 3 things if the entire project is completed, how do we present it to sir in github only, and what should we say" | Found that `performance.csv` and `report.md` were empty, `correctness.csv` held stale scores, the README mentioned a class that did not exist, and the 4-thread timings showed the parallel code slower than sequential. Used the list to assign fixes to each member. | Findings were checked by re-cloning the repo, compiling, and re-running the sequential and parallel code on all datasets. The stale scores were confirmed (see `docs/Critical_Evaluation.md`, case 3). |
| P3 | Review after switching to C++ | "https://github.com/pawarpranjali77/DAA-Project-T1 check this now we have tried making it in cpp but some parts in java because my other class mates have made it in cpp only" | Found that the committed `performance.csv` came from the earlier Java run on a 1-core machine, that the report still said Java, and that binaries and helper scripts were committed. | Rebuilt with `make`, re-ran `CorrectnessTester` (all PASS) and the C++ benchmark. Timings differed from the committed file, which confirmed the provenance problem (case 4). Results were then regenerated on the Apple M4. |
| P4 | Improvement planning | "can you give prompts to improve the whole project and make A tier project give 5 lines ke 3 prompts to solve all the problems" | Received 3 suggested prompts: (1) clean and flatten the repo, (2) fix the slow parallel code and re-benchmark, (3) finish the report, README and viva notes. TODO: record which of these were actually run and what was changed. | TODO: state how each resulting change was verified (e.g. `CorrectnessTester` all PASS, benchmark re-run). |
| P5 | Requirement compliance check | "now tell if the project is according to this requiremnet some common requirements but like we are goibg to inlcude few things in the report as well but based on htis how much would you rate the project give feedback as well Q3. Biological Sequence Alignment" (the course instruction sheet was attached) | Identified the gaps against the instruction sheet: no OpenMP/MPI/CUDA, missing LLM log, missing individual contribution statements. | Each gap was checked by hand against the instruction sheet. TODO: record what was done for each gap. |

## Fatima: parallel implementation

| Sr. No. | Purpose | Prompt Used | How the Response Was Used | Modification / Verification |
|---|---|---|---|---|
| F1 | Parallelization: wavefront / anti-diagonal approach | TODO: paste actual prompt | TODO | TODO |
| F2 | Optimization: reducing barrier cost (tiled / blocked wavefront) | TODO: paste actual prompt | TODO | Tile size 128; compared experimentally. Size 10000, 8 threads: 2.757x speedup. |
| F3 | Debugging | TODO: paste actual prompt, or delete this row | TODO | TODO |

## Maitreyi: performance analysis

| Sr. No. | Purpose | Prompt Used | How the Response Was Used | Modification / Verification |
|---|---|---|---|---|
| Y1 | Benchmark design (warm-up, median of runs, thread counts) | TODO: paste actual prompt | TODO | TODO |
| Y2 | Graph generation (`performance.py`) | TODO: paste actual prompt | TODO | TODO |
| Y3 | Analysis of bottlenecks / why speedup is not linear | TODO: paste actual prompt | TODO | TODO |

---

## Critical evaluation of LLM output

At least one LLM suggestion was rejected or modified. The full write-up (what was
suggested, why it was considered, what was changed, how it was verified) is in
[`docs/Critical_Evaluation.md`](docs/Critical_Evaluation.md). Summary:

1. Row-wise parallelization: rejected (cell dependencies).
2. One barrier per anti-diagonal: replaced by a tiled wavefront after measuring a slowdown.
3. Stale `correctness.csv`: rejected after a score that was impossible for the scoring scheme.
4. Benchmark results from the wrong run (Java, 1 core): discarded and re-measured.

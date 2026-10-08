# Viva Notes

## 1. Why not parallelize row-by-row directly?

Because the Needleman-Wunsch recurrence depends on the previous row and previous column values. A cell at `(i, j)` depends on `(i-1, j)`, `(i, j-1)` and `(i-1, j-1)`. This makes independent row execution invalid unless the required dependencies are synchronized carefully. The wavefront strategy keeps the dependency order intact while allowing cells on the same anti-diagonal to run in parallel.

## 2. What is the time complexity of the sequential version?

The standard full-matrix dynamic-programming algorithm runs in $O(nm)$ time and uses $O(nm)$ memory if the entire matrix is stored for traceback. In this project, the score-only version stores only a rolling buffer and therefore reduces memory use while keeping the same asymptotic computation cost.

## 3. What is the complexity of the parallel version?

The total work remains $O(nm)$, but the wall-clock time is reduced by parallelism when the work is large enough. In practice, the parallel runtime is bounded by synchronization cost, memory access and under-utilized short diagonals.

## 4. Why use `std::thread` instead of OpenMP?

`std::thread` gives explicit control over thread creation, scheduling and work partitioning in a portable C++ manner. OpenMP is also valid, but for this project the custom blocked wavefront approach was clearer and easier to reason about in terms of dependency ordering and barrier management.

## 5. Why is speedup sometimes below 1?

When the matrix is small, the overhead from thread management, synchronization and memory traffic exceeds the actual work. This is visible in the real results: at size 100 and 500, 8-thread speedup is below 1, meaning the parallel version is slower than the single-thread version.

## 6. Why does the speedup improve for larger matrices?

As the matrix grows, the amount of useful computation per tile increases, which amortizes the synchronization cost. The best measured case in the real data is size 10000 with 8 threads, where speedup reaches 2.757x.

## 7. How was correctness verified?

Correctness was verified by comparing the sequential and parallel scores on all six dataset sizes and four thread counts. The output from `results/correctness.csv` showed exact matches for every case.

## 8. What is the exact proof that the parallel output is correct?

The project stores a result row for each size and thread count, including both sequential score and parallel score. All entries in the generated correctness file are `PASS`.

## 9. Why is the row-based sequential algorithm still important?

The row-based sequential algorithm remains the best single-thread implementation because it is cache-friendly and avoids synchronization overhead. It is therefore used as the baseline and also as the 1-thread path in the parallel implementation.

## 10. What are the bottlenecks in the wavefront approach?

The main bottlenecks are synchronization between tiles, short edge diagonals with little work, memory/cache behavior, and thread overhead. These effects dominate for small inputs.

## 11. What is Amdahl's law and why does it matter here?

Amdahl's law says that the overall speedup is limited by the serial fraction of the computation. In this project, even a large parallel component is limited by synchronization and frontier management, so ideal linear speedup is not achievable.

## 12. Why do we use blocked tiles instead of a simple diagonal barrier?

A blocked design reduces synchronization frequency by processing many cells per tile before waiting. This amortizes the cost of barrier coordination and makes the algorithm more efficient than a barrier per anti-diagonal.

## 13. Why is the speedup not linear even when the algorithm is parallel?

Because the algorithm is not perfectly parallel. Some work remains synchronization-bound and some work is spent on memory access, cache miss penalties and thread coordination. That is why speedup is sub-linear even on 10 cores.

## 14. Why were the graphs generated from the actual performance CSV?

Because the project should be reproducible and evidence-based. The graphs are generated directly from `results/performance.csv`, so they reflect real measured values rather than synthetic or estimated numbers.

## 15. What is the main takeaway from this project?

The project demonstrates that a correct parallel implementation of Needleman-Wunsch is feasible, and that a blocked wavefront can improve performance on larger datasets. However, the algorithm remains limited by synchronization and memory behavior, so speedup increases with input size but is still below ideal scaling for small problems.

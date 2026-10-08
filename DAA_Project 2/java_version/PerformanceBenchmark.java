import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.Arrays;

/**
 * Benchmarks sequential vs parallel Needleman-Wunsch on the SAME dataset
 * pairs that CorrectnessTester uses (datasets/data_<size>_A.txt / _B.txt),
 * for every size at 1, 2, 4 and 8 threads.
 *
 * Writes:
 *   results/performance.csv          (one row per size x threads)
 *   results/benchmark_environment.txt (cores, JVM, OS, timing method)
 *
 * It also cross-checks that the sequential score equals the full-matrix
 * score from NeedlemanWunschSequential, so the scores in performance.csv
 * are directly comparable with results/correctness.csv.
 *
 * Run from the project root:
 *   java -cp bin PerformanceBenchmark
 */
public class PerformanceBenchmark {

    static final int[] SIZES = {100, 500, 1000, 2000, 5000, 10000};
    static final int[] THREADS = {1, 2, 4, 8};
    static final int WARMUP = 5;
    static final int REPS = 7;

    static double medianMs(long[] t) {
        long[] c = t.clone();
        Arrays.sort(c);
        return c[c.length / 2] / 1e6;
    }

    public static void main(String[] args) throws Exception {

        int cores = Runtime.getRuntime().availableProcessors();

        try (PrintWriter env = new PrintWriter(
                new FileWriter("results/benchmark_environment.txt"))) {
            env.println("cores_available=" + cores);
            env.println("java=" + System.getProperty("java.version")
                    + " (" + System.getProperty("java.vm.name") + ")");
            env.println("os=" + System.getProperty("os.name") + " "
                    + System.getProperty("os.version") + " "
                    + System.getProperty("os.arch"));
            env.println("inputs=datasets/data_<size>_A.txt, "
                    + "datasets/data_<size>_B.txt (same as CorrectnessTester)");
            env.println("timing=median of " + REPS + " runs after a global "
                    + "JIT warm-up and " + WARMUP
                    + " per-config warm-up runs, System.nanoTime()");
            env.println("baseline=NeedlemanWunschParallel.sequentialScore "
                    + "(row-by-row, score only)");
            env.println("min_parallel_len="
                    + NeedlemanWunschParallel.MIN_PARALLEL_LEN);
        }

        System.out.println("Cores available: " + cores);
        if (cores < 8) {
            System.out.println("WARNING: fewer than 8 cores; thread counts "
                    + "above " + cores + " are oversubscribed and their "
                    + "speedups are not meaningful.");
        }

        // Global JIT warm-up so no size is measured before the hot loops
        // are compiled (otherwise the first sizes get an unfair baseline).
        {
            byte[] wa = CorrectnessTester.readSequence(
                    "datasets/data_2000_A.txt").getBytes();
            byte[] wb = CorrectnessTester.readSequence(
                    "datasets/data_2000_B.txt").getBytes();
            for (int i = 0; i < 20; i++) {
                NeedlemanWunschParallel.sequentialScore(wa, wb);
            }
            for (int th : THREADS) {
                for (int i = 0; i < 3; i++) {
                    NeedlemanWunschParallel.parallelScore(wa, wb, th);
                }
            }
        }

        try (PrintWriter out = new PrintWriter(
                new FileWriter("results/performance.csv"))) {

            out.println("Size,Threads,Sequential Time ms,Parallel Time ms,"
                    + "Speedup,Efficiency,Sequential Score,Parallel Score,"
                    + "Match");

            for (int size : SIZES) {

                String sa = CorrectnessTester.readSequence(
                        "datasets/data_" + size + "_A.txt");
                String sb = CorrectnessTester.readSequence(
                        "datasets/data_" + size + "_B.txt");

                byte[] a = sa.getBytes();
                byte[] b = sb.getBytes();

                int seqScore = NeedlemanWunschParallel.sequentialScore(a, b);
                int refScore = CorrectnessTester.getSequentialScore(sa, sb);
                if (seqScore != refScore) {
                    throw new IllegalStateException(
                            "Baseline mismatch at size " + size + ": "
                            + seqScore + " vs " + refScore);
                }

                for (int i = 0; i < WARMUP; i++) {
                    NeedlemanWunschParallel.sequentialScore(a, b);
                }
                long[] ts = new long[REPS];
                for (int r = 0; r < REPS; r++) {
                    long s = System.nanoTime();
                    NeedlemanWunschParallel.sequentialScore(a, b);
                    ts[r] = System.nanoTime() - s;
                }
                double seqMs = medianMs(ts);

                for (int th : THREADS) {

                    int score = 0;
                    for (int i = 0; i < WARMUP; i++) {
                        score = NeedlemanWunschParallel
                                .parallelScore(a, b, th);
                    }
                    long[] tp = new long[REPS];
                    for (int r = 0; r < REPS; r++) {
                        long s = System.nanoTime();
                        score = NeedlemanWunschParallel
                                .parallelScore(a, b, th);
                        tp[r] = System.nanoTime() - s;
                    }
                    double parMs = medianMs(tp);
                    double speedup = seqMs / parMs;

                    String row = String.format(
                            "%d,%d,%.3f,%.3f,%.3f,%.3f,%d,%d,%s",
                            size, th, seqMs, parMs, speedup,
                            speedup / th, seqScore, score,
                            seqScore == score ? "PASS" : "FAIL");
                    out.println(row);
                    out.flush();
                    System.out.println(row);
                }
            }
        }

        System.out.println("Saved results/performance.csv");
    }
}

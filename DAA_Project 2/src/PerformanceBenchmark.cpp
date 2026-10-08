#include "CorrectnessTester.h"
#include "NeedlemanWunschParallel.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#if defined(__linux__)
#include <sys/utsname.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#endif

/**
 * Benchmarks sequential vs parallel Needleman-Wunsch on the SAME dataset pairs
 * that CorrectnessTester uses (datasets/data_<size>_A.txt / _B.txt),
 * for every size at 1, 2, 4 and 8 threads.
 *
 * Writes:
 *   results/performance.csv           (one row per size x threads)
 *   results/benchmark_environment.txt (cores, compiler, OS, timing method)
 *
 * Run from the project root.
 */
static const int SIZES[] = {100, 500, 1000, 2000, 5000, 10000};
static const int THREADS[] = {1, 2, 4, 8};
static const int WARMUP = 5;
static const int REPS = 7;

static std::string cpuModel() {
#if defined(__linux__)
    struct utsname u;
    if (uname(&u) == 0) return std::string(u.machine) + " / " + u.release;
#elif defined(__APPLE__)
    char model[256];
    size_t size = sizeof(model);
    if (sysctlbyname("machdep.cpu.brand_string", model, &size, nullptr, 0) == 0) {
        return std::string(model);
    }
#endif
    return "unknown";
}

static std::string compilerFlags() {
    return "-std=c++17 -O2 -Wall -pthread";
}

static double medianMs(const std::vector<long long>& t) { return NWPar::medianMs(t); }

static long long nowNs() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::steady_clock::now().time_since_epoch()).count();
}

static std::vector<uint8_t> toBytes(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

int main() {
    try {
        unsigned cores = std::thread::hardware_concurrency();

        {
            std::ofstream env("results/benchmark_environment.txt");
            if (!env) throw std::runtime_error("Cannot write results/benchmark_environment.txt");
            env << "cores_available=" << cores << "\n";
            env << "cpu_model=" << cpuModel() << "\n";
            env << "compiler_flags=" << compilerFlags() << "\n";
#if defined(__VERSION__)
            env << "compiler=" << __VERSION__ << "\n";
#endif
#if defined(__linux__)
            struct utsname u;
            if (uname(&u) == 0)
                env << "os=" << u.sysname << " " << u.release << " " << u.machine << "\n";
#elif defined(__APPLE__)
            env << "os=macOS\n";
#endif
            env << "inputs=datasets/data_<size>_A.txt, datasets/data_<size>_B.txt "
                   "(same as CorrectnessTester)\n";
            env << "timing=median of " << REPS << " runs after a global warm-up and "
                << WARMUP << " per-config warm-up runs, std::chrono::steady_clock\n";
            env << "baseline=NWPar::sequentialScore (row-by-row, score only)\n";
            env << "tile_size=128\n";
            env << "min_parallel_len=" << NWPar::MIN_PARALLEL_LEN << "\n";
        }

        std::cout << "Cores available: " << cores << std::endl;
        if (cores < 8) {
            std::cout << "WARNING: fewer than 8 cores; thread counts above " << cores
                      << " are oversubscribed and their speedups are not meaningful." << std::endl;
        }

        // Global warm-up so no size is measured before caches/CPU are warm
        {
            auto wa = toBytes(CorrectnessTester::readSequence("datasets/data_2000_A.txt"));
            auto wb = toBytes(CorrectnessTester::readSequence("datasets/data_2000_B.txt"));
            for (int i = 0; i < 20; ++i) NWPar::sequentialScore(wa, wb);
            for (int th : THREADS)
                for (int i = 0; i < 3; ++i) NWPar::parallelScore(wa, wb, th);
        }

        std::ofstream out("results/performance.csv");
        if (!out) throw std::runtime_error("Cannot write results/performance.csv");

        out << "Size,Threads,Sequential Time ms,Parallel Time ms,Speedup,Efficiency,"
               "Sequential Score,Parallel Score,Match\n";

        for (int size : SIZES) {
            std::string sa = CorrectnessTester::readSequence(
                "datasets/data_" + std::to_string(size) + "_A.txt");
            std::string sb = CorrectnessTester::readSequence(
                "datasets/data_" + std::to_string(size) + "_B.txt");

            auto a = toBytes(sa);
            auto b = toBytes(sb);

            int seqScore = NWPar::sequentialScore(a, b);
            int refScore = CorrectnessTester::getSequentialScore(sa, sb);
            if (seqScore != refScore) {
                throw std::runtime_error("Baseline mismatch at size " + std::to_string(size) +
                                         ": " + std::to_string(seqScore) + " vs " +
                                         std::to_string(refScore));
            }

            for (int i = 0; i < WARMUP; ++i) NWPar::sequentialScore(a, b);
            std::vector<long long> ts(REPS);
            for (int r = 0; r < REPS; ++r) {
                long long s = nowNs();
                NWPar::sequentialScore(a, b);
                ts[r] = nowNs() - s;
            }
            double seqMs = medianMs(ts);

            for (int th : THREADS) {
                if (th > static_cast<int>(cores)) {
                    std::cout << "WARNING: requested threads=" << th
                              << " exceeds cores_available=" << cores
                              << "; benchmark still runs but speedup is not meaningful." << std::endl;
                }

                int score = 0;
                for (int i = 0; i < WARMUP; ++i) score = NWPar::parallelScore(a, b, th);
                std::vector<long long> tp(REPS);
                for (int r = 0; r < REPS; ++r) {
                    long long s = nowNs();
                    score = NWPar::parallelScore(a, b, th);
                    tp[r] = nowNs() - s;
                }
                double parMs = medianMs(tp);
                double speedup = seqMs / parMs;

                char row[256];
                std::snprintf(row, sizeof(row), "%d,%d,%.3f,%.3f,%.3f,%.3f,%d,%d,%s",
                              size, th, seqMs, parMs, speedup, speedup / th, seqScore, score,
                              seqScore == score ? "PASS" : "FAIL");
                out << row << std::endl;
                std::cout << row << std::endl;
            }
        }

        std::cout << "Saved results/performance.csv" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

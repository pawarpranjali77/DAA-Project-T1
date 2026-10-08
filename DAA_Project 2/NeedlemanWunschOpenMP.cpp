// OpenMP tiled-wavefront Needleman-Wunsch (score only).
//
// Same decomposition as NeedlemanWunschParallel.cpp (std::thread pool):
// the DP matrix is cut into tileSize x tileSize tiles; all tiles on one
// tile anti-diagonal are independent (they only read tiles on the previous
// diagonals), so each diagonal is a parallel loop. The implicit barrier at
// the end of `omp for` is the diagonal-to-diagonal synchronisation.
//
// Build:  make cpp_bin/NeedlemanWunschOpenMP
// Run:    ./cpp_bin/NeedlemanWunschOpenMP [size] [threads,comma,list]
//         ./cpp_bin/NeedlemanWunschOpenMP -f a.txt b.txt [threads,comma,list]
#include "NeedlemanWunschParallel.h"

#include <omp.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace NWOmp {

using NWPar::GAP;
using NWPar::MATCH;
using NWPar::MISMATCH;

int scoreTiled(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b,
               int threads, int tileSize) {
    const int n = static_cast<int>(a.size());
    const int m = static_cast<int>(b.size());
    if (n == 0 || m == 0) return (n + m) * GAP;

    const int rows = (n + tileSize - 1) / tileSize;
    const int cols = (m + tileSize - 1) / tileSize;
    const size_t W = static_cast<size_t>(m) + 1;

    std::vector<int> dp(static_cast<size_t>(n + 1) * W, 0);
    for (int i = 0; i <= n; ++i) dp[i * W] = i * GAP;
    for (int j = 0; j <= m; ++j) dp[j] = j * GAP;

    int* D = dp.data();
    const uint8_t* A = a.data();
    const uint8_t* B = b.data();
    const int numDiags = rows + cols - 1;

    #pragma omp parallel num_threads(threads) default(none) \
            shared(D, A, B) firstprivate(n, m, rows, cols, W, tileSize, numDiags)
    {
        for (int d = 0; d < numDiags; ++d) {
            const int rLo = std::max(0, d - (cols - 1));
            const int rHi = std::min(rows - 1, d);

            // Tiles (r, d-r) on this anti-diagonal are independent.
            // Implicit barrier at end of the loop = wavefront dependency.
            #pragma omp for schedule(dynamic, 1)
            for (int r = rLo; r <= rHi; ++r) {
                const int c = d - r;
                const int iBeg = r * tileSize + 1;
                const int iEnd = std::min(n, (r + 1) * tileSize);
                const int jBeg = c * tileSize + 1;
                const int jEnd = std::min(m, (c + 1) * tileSize);

                for (int i = iBeg; i <= iEnd; ++i) {
                    const int* up = D + (i - 1) * W;
                    int* cur = D + i * W;
                    for (int j = jBeg; j <= jEnd; ++j) {
                        const int s = up[j - 1] + (A[i - 1] == B[j - 1] ? MATCH : MISMATCH);
                        cur[j] = std::max(s, std::max(up[j] + GAP, cur[j - 1] + GAP));
                    }
                }
            }
        }
    }
    return dp[static_cast<size_t>(n) * W + m];
}

int score(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b, int threads) {
    if (threads <= 1) return NWPar::sequentialScore(a, b);
    const int n = static_cast<int>(a.size());
    const int m = static_cast<int>(b.size());
    const int tile = std::max(64, std::min(256, std::max(n, m) / 16));
    return scoreTiled(a, b, threads, tile);
}

} // namespace NWOmp

#ifdef NEEDLEMANWUNSCHOPENMP_MAIN
static long long nowNs() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::steady_clock::now().time_since_epoch()).count();
}

int main(int argc, char** argv) {
    using namespace NWPar;
    try {
        std::vector<uint8_t> a, b;
        int idx;
        if (argc - 1 >= 3 && std::string(argv[1]) == "-f") {
            a = readSeq(argv[2]);
            b = readSeq(argv[3]);
            idx = 4;
        } else {
            int size = (argc - 1 > 0) ? std::stoi(argv[1]) : 1000;
            a = randomDNA(size, 42);
            b = randomDNA(size, 4242);
            idx = 2;
        }

        std::vector<int> threadList = {1, 2, 4, 8};
        if (argc > idx) {
            threadList.clear();
            std::stringstream ss(argv[idx]);
            std::string tok;
            while (std::getline(ss, tok, ',')) threadList.push_back(std::stoi(tok));
        }

        const int seqScore = sequentialScore(a, b);
        std::vector<long long> ts(REPS);
        for (int r = 0; r < REPS; ++r) {
            long long s = nowNs();
            sequentialScore(a, b);
            ts[r] = nowNs() - s;
        }
        const double seqMs = medianMs(ts);

        std::printf("OpenMP version (omp_get_max_threads=%d)\n", omp_get_max_threads());
        std::printf("Sequence lengths: %d x %d\n", (int)a.size(), (int)b.size());
        std::printf("Sequential score = %d, time = %.3f ms\n", seqScore, seqMs);
        std::printf("size,threads,seq_ms,omp_ms,speedup,efficiency,seq_score,omp_score,match\n");

        bool allOk = true;
        for (int th : threadList) {
            int sc = NWOmp::score(a, b, th);
            std::vector<long long> tp(REPS);
            for (int r = 0; r < REPS; ++r) {
                long long s = nowNs();
                sc = NWOmp::score(a, b, th);
                tp[r] = nowNs() - s;
            }
            const double ms = medianMs(tp);
            const double sp = seqMs / ms;
            allOk &= (sc == seqScore);
            std::printf("%d,%d,%.3f,%.3f,%.3f,%.3f,%d,%d,%s\n", (int)a.size(), th, seqMs, ms,
                        sp, sp / th, seqScore, sc, sc == seqScore ? "PASS" : "FAIL");
        }
        return allOk ? 0 : 2;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
#endif

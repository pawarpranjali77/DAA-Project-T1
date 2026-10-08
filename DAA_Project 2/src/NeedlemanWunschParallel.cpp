#include "NeedlemanWunschParallel.h"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <stdexcept>
#include <thread>

/**
 * Parallel Needleman-Wunsch (score only) using wavefront / anti-diagonal processing.
 *
 * D[i][j] needs D[i-1][j-1], D[i-1][j], D[i][j-1]. All three lie on diagonals
 * d-2 and d-1 (d = i+j), so every cell on diagonal d is independent of the
 * others on diagonal d. A barrier between diagonals enforces dependency order.
 *
 * Memory: only 3 diagonal buffers (O(n)) are kept.
 *
 * Usage:
 *   NeedlemanWunschParallel <size> [t1,t2,...]
 *   NeedlemanWunschParallel -f seqA.txt seqB.txt [t1,t2,...]
 */
namespace NWPar {

namespace {

// Reusable cyclic barrier (equivalent of java.util.concurrent.CyclicBarrier)
class CyclicBarrier {
public:
    explicit CyclicBarrier(int parties) : parties_(parties), waiting_(0), generation_(0) {}

    void await() {
        std::unique_lock<std::mutex> lk(m_);
        int gen = generation_;
        if (++waiting_ == parties_) {
            generation_++;
            waiting_ = 0;
            cv_.notify_all();
        } else {
            cv_.wait(lk, [&] { return gen != generation_; });
        }
    }

private:
    std::mutex m_;
    std::condition_variable cv_;
    int parties_;
    int waiting_;
    int generation_;
};

} // namespace

// ---------- Reference sequential (row-by-row, full matrix-free) ----------
int sequentialScore(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    int n = (int)a.size();
    int m = (int)b.size();

    std::vector<int> prev(m + 1), cur(m + 1);
    for (int j = 0; j <= m; j++) prev[j] = j * GAP;

    for (int i = 1; i <= n; i++) {
        cur[0] = i * GAP;
        for (int j = 1; j <= m; j++) {
            int s = prev[j - 1] + (a[i - 1] == b[j - 1] ? MATCH : MISMATCH);
            cur[j] = std::max(s, std::max(prev[j] + GAP, cur[j - 1] + GAP));
        }
        std::swap(prev, cur);
    }
    return prev[m];
}

// ---------- Parallel wavefront ----------
int parallelScore(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b, int threads) {
    const int n = (int)a.size();
    const int m = (int)b.size();

    if (threads == 1) return wavefrontSingle(a, b);

    std::vector<int> bufs[3] = {
        std::vector<int>(n + 2, 0),
        std::vector<int>(n + 2, 0),
        std::vector<int>(n + 2, 0)};

    // diagonal 0: D[0][0]
    bufs[0][0] = 0;

    CyclicBarrier barrier(threads);
    const int lastD = n + m;

    auto worker = [&](int tid) {
        // Each thread rotates its own references identically
        int* pp = bufs[0].data();
        int* p = bufs[1].data();
        int* c = bufs[2].data();

        // Diagonal 1 boundaries
        if (tid == 0) {
            if (m >= 1) p[0] = GAP;
            if (n >= 1) p[1] = GAP;
        }
        barrier.await();

        for (int d = 2; d <= lastD; d++) {
            int lo = std::max(1, d - m);
            int hi = std::min(n, d - 1);

            // Boundary cells
            if (tid == 0) {
                if (d <= m) c[0] = d * GAP;
                if (d <= n) c[d] = d * GAP;
            }

            int len = hi - lo + 1;
            if (len > 0) {
                int from, to;
                if (len < MIN_PARALLEL_LEN) {
                    if (tid == 0) { from = lo; to = hi; }
                    else { from = 1; to = 0; }
                } else {
                    from = lo + (int)((long long)len * tid / threads);
                    to = lo + (int)((long long)len * (tid + 1) / threads) - 1;
                }

                for (int i = from; i <= to; i++) {
                    int j = d - i;
                    int s = pp[i - 1] + (a[i - 1] == b[j - 1] ? MATCH : MISMATCH);
                    c[i] = std::max(s, std::max(p[i - 1] + GAP, p[i] + GAP));
                }
            }

            // All cells of diagonal d must finish first
            barrier.await();

            int* tmp = pp;
            pp = p;
            p = c;
            c = tmp;
        }
    };

    std::vector<std::thread> pool;
    pool.reserve(threads);
    for (int t = 0; t < threads; t++) pool.emplace_back(worker, t);
    for (auto& th : pool) th.join();

    // Diagonal d is stored in bufs[d % 3]
    return bufs[lastD % 3][n];
}

// 1-thread version of the same diagonal algorithm
int wavefrontSingle(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    int n = (int)a.size();
    int m = (int)b.size();

    std::vector<int> bufs[3] = {
        std::vector<int>(n + 2, 0),
        std::vector<int>(n + 2, 0),
        std::vector<int>(n + 2, 0)};

    bufs[0][0] = 0;
    if (m >= 1) bufs[1][0] = GAP;
    if (n >= 1) bufs[1][1] = GAP;

    int* pp = bufs[0].data();
    int* p = bufs[1].data();
    int* c = bufs[2].data();

    for (int d = 2; d <= n + m; d++) {
        if (d <= m) c[0] = d * GAP;
        if (d <= n) c[d] = d * GAP;

        int lo = std::max(1, d - m);
        int hi = std::min(n, d - 1);

        for (int i = lo; i <= hi; i++) {
            int j = d - i;
            int s = pp[i - 1] + (a[i - 1] == b[j - 1] ? MATCH : MISMATCH);
            c[i] = std::max(s, std::max(p[i - 1] + GAP, p[i] + GAP));
        }

        int* tmp = pp;
        pp = p;
        p = c;
        c = tmp;
    }
    return bufs[(n + m) % 3][n];
}

// ---------- Helpers ----------
std::vector<uint8_t> randomDNA(int len, uint64_t seed) {
    std::mt19937_64 r(seed);
    std::vector<uint8_t> s(len);
    const uint8_t alpha[] = {'A', 'T', 'C', 'G'};
    for (int i = 0; i < len; i++) s[i] = alpha[r() % 4];
    return s;
}

std::vector<uint8_t> readSeq(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot open file: " + path);
    std::stringstream ss;
    ss << in.rdbuf();
    std::string raw = ss.str();

    std::string s;
    for (char ch : raw) {
        if (!std::isspace((unsigned char)ch)) s.push_back((char)std::toupper((unsigned char)ch));
    }
    if (!s.empty() && s[0] == '>')
        throw std::runtime_error("FASTA headers not supported: " + path);

    return std::vector<uint8_t>(s.begin(), s.end());
}

double medianMs(std::vector<long long> t) {
    std::sort(t.begin(), t.end());
    return t[t.size() / 2] / 1e6;
}

} // namespace NWPar

#ifdef NEEDLEMANWUNSCHPARALLEL_MAIN
using namespace NWPar;

static long long nowNs() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::steady_clock::now().time_since_epoch()).count();
}

int main(int argc, char** argv) {
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

        // Sequential baseline: warm-up + timed
        int seqScore = sequentialScore(a, b);
        std::vector<long long> ts(REPS);
        for (int r = 0; r < REPS; r++) {
            long long s = nowNs();
            sequentialScore(a, b);
            ts[r] = nowNs() - s;
        }
        double seqMs = medianMs(ts);

        std::printf("Sequence lengths: %d x %d, cores available: %u\n",
                    (int)a.size(), (int)b.size(), std::thread::hardware_concurrency());
        std::printf("Sequential score = %d, time = %.3f ms\n", seqScore, seqMs);
        std::printf("size,threads,seq_ms,par_ms,speedup,efficiency,seq_score,par_score,match\n");

        for (int th : threadList) {
            // Warm-up
            int score = parallelScore(a, b, th);

            std::vector<long long> tp(REPS);
            for (int r = 0; r < REPS; r++) {
                long long s = nowNs();
                score = parallelScore(a, b, th);
                tp[r] = nowNs() - s;
            }
            double parMs = medianMs(tp);
            double speedup = seqMs / parMs;

            std::printf("%d,%d,%.3f,%.3f,%.3f,%.3f,%d,%d,%s\n",
                        (int)a.size(), th, seqMs, parMs, speedup, speedup / th,
                        seqScore, score, seqScore == score ? "PASS" : "FAIL");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
#endif

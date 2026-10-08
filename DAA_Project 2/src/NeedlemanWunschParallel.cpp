#include "NeedlemanWunschParallel.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <unordered_map>

namespace NWPar {

namespace {

struct PersistentThreadPool {
    explicit PersistentThreadPool(int workerCount)
        : workerCount_(workerCount) {
        workers_.reserve(workerCount_);
        for (int i = 0; i < workerCount_; ++i) {
            workers_.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lk(mutex_);
                        cv_.wait(lk, [this] { return stop_ || !queue_.empty(); });
                        if (stop_ && queue_.empty()) return;
                        task = std::move(queue_.front());
                        queue_.pop();
                    }
                    task();
                    {
                        std::lock_guard<std::mutex> lk(mutex_);
                        --remaining_;
                    }
                    cv_done_.notify_one();
                }
            });
        }
    }

    ~PersistentThreadPool() {
        {
            std::lock_guard<std::mutex> lk(mutex_);
            stop_ = true;
        }
        cv_.notify_all();
        for (auto& worker : workers_) worker.join();
    }

    void execute(const std::vector<std::function<void()>>& tasks) {
        if (tasks.empty()) return;

        {
            std::lock_guard<std::mutex> lk(mutex_);
            remaining_ = static_cast<int>(tasks.size());
            for (const auto& task : tasks) queue_.push(task);
        }
        cv_.notify_all();

        std::unique_lock<std::mutex> lk(mutex_);
        cv_done_.wait(lk, [this] { return remaining_ == 0; });
    }

private:
    int workerCount_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::condition_variable cv_done_;
    std::queue<std::function<void()>> queue_;
    std::vector<std::thread> workers_;
    int remaining_ = 0;
    bool stop_ = false;
};

PersistentThreadPool& getThreadPool(int threads) {
    static std::mutex mapMutex;
    static std::unordered_map<int, std::unique_ptr<PersistentThreadPool>> pools;

    std::lock_guard<std::mutex> lock(mapMutex);
    auto it = pools.find(threads);
    if (it == pools.end()) {
        auto pool = std::make_unique<PersistentThreadPool>(threads);
        it = pools.emplace(threads, std::move(pool)).first;
    }
    return *it->second;
}

int parallelScoreBlocked(const std::vector<uint8_t>& a,
                        const std::vector<uint8_t>& b,
                        int threads,
                        int tileSize) {
    const int n = static_cast<int>(a.size());
    const int m = static_cast<int>(b.size());
    if (n == 0 || m == 0) return 0;

    int rows = (n + tileSize - 1) / tileSize;
    int cols = (m + tileSize - 1) / tileSize;

    std::vector<int> dp((n + 1) * (m + 1), 0);
    for (int i = 0; i <= n; ++i) dp[i * (m + 1)] = i * GAP;
    for (int j = 0; j <= m; ++j) dp[j] = j * GAP;

    auto computeTile = [&](int tileRow, int tileCol) {
        const int rowStart = tileRow * tileSize + 1;
        const int colStart = tileCol * tileSize + 1;
        const int rowEnd = std::min(n, (tileRow + 1) * tileSize);
        const int colEnd = std::min(m, (tileCol + 1) * tileSize);

        for (int i = rowStart; i <= rowEnd; ++i) {
            const int rowBase = i * (m + 1);
            for (int j = colStart; j <= colEnd; ++j) {
                const int diag = dp[(i - 1) * (m + 1) + (j - 1)];
                const int up = dp[(i - 1) * (m + 1) + j];
                const int left = dp[rowBase + (j - 1)];
                const int matchScore = diag + (a[i - 1] == b[j - 1] ? MATCH : MISMATCH);
                dp[rowBase + j] = std::max(matchScore, std::max(up + GAP, left + GAP));
            }
        }
    };

    for (int tileDiag = 0; tileDiag < rows + cols - 1; ++tileDiag) {
        std::vector<std::function<void()>> tasks;
        tasks.reserve(8);

        for (int r = 0; r < rows; ++r) {
            int c = tileDiag - r;
            if (c < 0 || c >= cols) continue;
            tasks.emplace_back([&, r, c] { computeTile(r, c); });
        }

        if (!tasks.empty()) {
            if (threads <= 1) {
                for (const auto& task : tasks) task();
            } else {
                getThreadPool(threads).execute(tasks);
            }
        }
    }

    return dp[n * (m + 1) + m];
}

} // namespace

// ---------- Reference sequential (row-by-row, full matrix-free) ----------
int sequentialScore(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    int n = static_cast<int>(a.size());
    int m = static_cast<int>(b.size());

    std::vector<int> prev(m + 1), cur(m + 1);
    for (int j = 0; j <= m; ++j) prev[j] = j * GAP;

    for (int i = 1; i <= n; ++i) {
        cur[0] = i * GAP;
        for (int j = 1; j <= m; ++j) {
            int s = prev[j - 1] + (a[i - 1] == b[j - 1] ? MATCH : MISMATCH);
            cur[j] = std::max(s, std::max(prev[j] + GAP, cur[j - 1] + GAP));
        }
        std::swap(prev, cur);
    }
    return prev[m];
}

// ---------- Parallel blocked wavefront ----------
int parallelScore(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b, int threads) {
    if (threads <= 1) return wavefrontSingle(a, b);

    const int n = static_cast<int>(a.size());
    const int m = static_cast<int>(b.size());
    const int tileSize = std::max(64, std::min(256, std::max(n, m) / 16));
    return parallelScoreBlocked(a, b, threads, tileSize);
}

// 1-thread version uses the row-by-row DP path, not the blocked wavefront.
int wavefrontSingle(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    return sequentialScore(a, b);
}

std::vector<std::pair<int, double>> tileSizeExperiment(const std::vector<uint8_t>& a,
                                                     const std::vector<uint8_t>& b,
                                                     int threads,
                                                     const std::vector<int>& tileSizes) {
    std::vector<std::pair<int, double>> results;
    results.reserve(tileSizes.size());

    std::vector<long long> times;
    times.reserve(5);

    for (int tile : tileSizes) {
        int minTile = std::max(32, tile);
        times.clear();
        for (int rep = 0; rep < 5; ++rep) {
            auto start = std::chrono::steady_clock::now();
            (void)parallelScoreBlocked(a, b, threads, minTile);
            auto end = std::chrono::steady_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            times.push_back(ms);
        }
        std::sort(times.begin(), times.end());
        double medianMs = static_cast<double>(times[times.size() / 2]) / 1000.0;
        results.emplace_back(minTile, medianMs);
    }
    return results;
}

// ---------- Helpers ----------
std::vector<uint8_t> randomDNA(int len, uint64_t seed) {
    std::mt19937_64 r(seed);
    std::vector<uint8_t> s(len);
    const uint8_t alpha[] = {'A', 'T', 'C', 'G'};
    for (int i = 0; i < len; ++i) s[i] = alpha[r() % 4];
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

        const int seqScore = sequentialScore(a, b);
        std::vector<long long> ts(REPS);
        for (int r = 0; r < REPS; ++r) {
            long long s = nowNs();
            sequentialScore(a, b);
            ts[r] = nowNs() - s;
        }
        const double seqMs = medianMs(ts);

        std::printf("Sequence lengths: %d x %d, cores available: %u\n",
                    (int)a.size(), (int)b.size(), std::thread::hardware_concurrency());
        std::printf("Sequential score = %d, time = %.3f ms\n", seqScore, seqMs);

        std::vector<int> tileSizes = {32, 64, 128, 256};
        std::printf("Tile experiment (threads=%d):\n", threadList.front());
        for (const auto& [tile, ms] : tileSizeExperiment(a, b, threadList.front(), tileSizes)) {
            std::printf("tile=%d, median_ms=%.3f\n", tile, ms);
        }

        std::printf("size,threads,seq_ms,par_ms,speedup,efficiency,seq_score,par_score,match\n");
        for (int th : threadList) {
            int score = parallelScore(a, b, th);
            std::vector<long long> tp(REPS);
            for (int r = 0; r < REPS; ++r) {
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

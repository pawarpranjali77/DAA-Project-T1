// Needleman-Wunsch: match +1, mismatch -1, gap -2
#pragma once
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <random>

constexpr int MATCH = 1, MISMATCH = -1, GAP = -2;
inline int sc(char a, char b) { return a == b ? MATCH : MISMATCH; }

struct Alignment { int score; std::string a, b; };

// Sequential: full DP matrix + traceback
inline Alignment nwAlign(const std::string& s, const std::string& t) {
    int n = s.size(), m = t.size();
    std::vector<std::vector<int>> H(n + 1, std::vector<int>(m + 1));
    for (int i = 0; i <= n; i++) H[i][0] = i * GAP;
    for (int j = 0; j <= m; j++) H[0][j] = j * GAP;
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= m; j++)
            H[i][j] = std::max({H[i-1][j-1] + sc(s[i-1], t[j-1]), H[i-1][j] + GAP, H[i][j-1] + GAP});
    std::string ra, rb;
    int i = n, j = m;
    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && H[i][j] == H[i-1][j-1] + sc(s[i-1], t[j-1])) { ra += s[--i]; rb += t[--j]; }
        else if (i > 0 && H[i][j] == H[i-1][j] + GAP) { ra += s[--i]; rb += '-'; }
        else { ra += '-'; rb += t[--j]; }
    }
    std::reverse(ra.begin(), ra.end()); std::reverse(rb.begin(), rb.end());
    return {H[n][m], ra, rb};
}

// Sequential score only, O(m) memory (used for large inputs)
inline int nwScoreSeq(const std::string& s, const std::string& t) {
    int n = s.size(), m = t.size();
    std::vector<int> prev(m + 1), cur(m + 1);
    for (int j = 0; j <= m; j++) prev[j] = j * GAP;
    for (int i = 1; i <= n; i++) {
        cur[0] = i * GAP;
        for (int j = 1; j <= m; j++)
            cur[j] = std::max({prev[j-1] + sc(s[i-1], t[j-1]), prev[j] + GAP, cur[j-1] + GAP});
        std::swap(prev, cur);
    }
    return prev[m];
}

class Barrier {
    std::mutex mu; std::condition_variable cv; int total, count = 0, gen = 0;
public:
    explicit Barrier(int n) : total(n) {}
    void wait() {
        std::unique_lock<std::mutex> lk(mu);
        int g = gen;
        if (++count == total) { count = 0; gen++; cv.notify_all(); }
        else cv.wait(lk, [&] { return g != gen; });
    }
};

// Parallel score: wavefront over anti-diagonals (i + j = d)
inline int nwScorePar(const std::string& s, const std::string& t, int threads) {
    int n = s.size(), m = t.size();
    if (threads < 1) threads = 1;
    std::vector<int> D[3] = {std::vector<int>(n + 1), std::vector<int>(n + 1), std::vector<int>(n + 1)};
    D[0][0] = 0;  // d = 0
    Barrier bar(threads);
    auto work = [&](int id) {
        for (int d = 1; d <= n + m; d++) {
            std::vector<int>& cur = D[d % 3];
            const std::vector<int>& p1 = D[(d + 2) % 3];  // d-1
            const std::vector<int>& p2 = D[(d + 1) % 3];  // d-2
            if (id == 0) {
                if (d <= n) cur[d] = d * GAP;   // (d,0)
                if (d <= m) cur[0] = d * GAP;   // (0,d)
            }
            int lo = std::max(1, d - m), hi = std::min(n, d - 1);
            if (lo <= hi) {
                int len = hi - lo + 1, chunk = (len + threads - 1) / threads;
                int a = lo + id * chunk, b = std::min(hi, a + chunk - 1);
                for (int i = a; i <= b; i++) {
                    int j = d - i;
                    cur[i] = std::max({p2[i-1] + sc(s[i-1], t[j-1]), p1[i-1] + GAP, p1[i] + GAP});
                }
            }
            bar.wait();
        }
    };
    std::vector<std::thread> th;
    for (int i = 1; i < threads; i++) th.emplace_back(work, i);
    work(0);
    for (auto& x : th) x.join();
    return D[(n + m) % 3][n];
}

inline std::string randomDNA(int len, unsigned seed) {
    std::mt19937 g(seed); const char* a = "ACGT"; std::string r(len, 'A');
    for (auto& c : r) c = a[g() % 4];
    return r;
}
inline std::string readSeqFile(const std::string& path) {
    std::ifstream f(path); std::string s, line;
    while (std::getline(f, line)) for (char c : line) if (c=='A'||c=='C'||c=='G'||c=='T') s += c;
    return s;
}
template <class F> double timeMs(F fn) {
    auto t0 = std::chrono::steady_clock::now(); fn();
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
}
template <class F> double bestMs(F fn, int reps = 3) {
    double b = 1e18; for (int i = 0; i < reps; i++) b = std::min(b, timeMs(fn)); return b;
}
inline const std::vector<int>& DATA_LENGTHS() { static std::vector<int> v{100,500,1000,2000,5000,10000}; return v; }
inline const std::vector<int>& THREADS() { static std::vector<int> v{1,2,4,8}; return v; }
inline std::string dataPath(int len, char ab) { return "datasets/data_" + std::to_string(len) + "_" + ab + ".txt"; }

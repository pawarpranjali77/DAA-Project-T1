#include "NeedlemanWunschSequential.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>

/**
 * Sequential Needleman-Wunsch global sequence alignment.
 *
 * Time : O(n * m)
 * Space : O(n * m) (full DP matrix kept for traceback)
 *
 * Usage:
 *   NeedlemanWunschSequential            -> runs small DNA tests
 *   NeedlemanWunschSequential <len1> <len2>
 *                                        -> random DNA benchmark (no alignment print)
 */
namespace NWSeq {

int score(char a, char b) {
    return (a == b) ? MATCH : MISMATCH;
}

// Fills the DP matrix. D[i][j] = best score aligning s1[0..i) with s2[0..j).
std::vector<std::vector<int>> buildMatrix(const std::string& s1, const std::string& s2) {
    int n = (int)s1.length();
    int m = (int)s2.length();

    std::vector<std::vector<int>> D(n + 1, std::vector<int>(m + 1, 0));

    // Initialization: aligning a prefix against an empty string = all gaps
    for (int i = 0; i <= n; i++) D[i][0] = i * GAP;
    for (int j = 0; j <= m; j++) D[0][j] = j * GAP;

    // Recurrence
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            int diag = D[i - 1][j - 1] + score(s1[i - 1], s2[j - 1]);
            int up = D[i - 1][j] + GAP;
            int left = D[i][j - 1] + GAP;
            D[i][j] = std::max(diag, std::max(up, left));
        }
    }
    return D;
}

// Traceback from D[n][m] to D[0][0]. Tie-break priority: diagonal, up, left.
Result traceback(const std::string& s1, const std::string& s2,
                 const std::vector<std::vector<int>>& D) {
    std::string a1, a2;
    int i = (int)s1.length();
    int j = (int)s2.length();

    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 &&
            D[i][j] == D[i - 1][j - 1] + score(s1[i - 1], s2[j - 1])) {
            a1.push_back(s1[i - 1]);
            a2.push_back(s2[j - 1]);
            i--;
            j--;
        } else if (i > 0 && D[i][j] == D[i - 1][j] + GAP) {
            a1.push_back(s1[i - 1]);
            a2.push_back('-');
            i--;
        } else {
            a1.push_back('-');
            a2.push_back(s2[j - 1]);
            j--;
        }
    }

    Result r;
    r.score = D[s1.length()][s2.length()];
    std::reverse(a1.begin(), a1.end());
    std::reverse(a2.begin(), a2.end());
    r.aligned1 = a1;
    r.aligned2 = a2;
    return r;
}

Result align(const std::string& s1, const std::string& s2) {
    return traceback(s1, s2, buildMatrix(s1, s2));
}

void printMatrix(const std::string& s1, const std::string& s2,
                 const std::vector<std::vector<int>>& D) {
    std::printf("          -");
    for (char c : s2) std::printf("%4c", c);
    std::printf("\n");

    for (int i = 0; i <= (int)s1.length(); i++) {
        if (i == 0) std::printf(" -");
        else std::printf(" %c", s1[i - 1]);
        for (int j = 0; j <= (int)s2.length(); j++) std::printf("%4d", D[i][j]);
        std::printf("\n");
    }
}

std::string matchLine(const std::string& a, const std::string& b) {
    std::string sb;
    for (size_t k = 0; k < a.length(); k++) {
        char x = a[k];
        char y = b[k];
        sb.push_back((x == '-' || y == '-') ? ' ' : (x == y ? '|' : '.'));
    }
    return sb;
}

void runTest(const std::string& name, const std::string& s1, const std::string& s2,
             bool showMatrix) {
    std::cout << "=== " << name << " ===" << std::endl;
    std::cout << "Seq1: " << s1 << std::endl;
    std::cout << "Seq2: " << s2 << std::endl;

    auto D = buildMatrix(s1, s2);
    if (showMatrix) printMatrix(s1, s2, D);

    Result r = traceback(s1, s2, D);
    std::cout << "Alignment score: " << r.score << std::endl;
    std::cout << r.aligned1 << std::endl;
    std::cout << matchLine(r.aligned1, r.aligned2) << std::endl;
    std::cout << r.aligned2 << std::endl;
    std::cout << std::endl;
}

std::string randomDNA(int len, std::mt19937_64& rnd) {
    const char bases[] = {'A', 'C', 'G', 'T'};
    std::string sb;
    sb.reserve(len);
    for (int i = 0; i < len; i++) sb.push_back(bases[rnd() % 4]);
    return sb;
}

} // namespace NWSeq

#ifdef NEEDLEMANWUNSCHSEQUENTIAL_MAIN
using namespace NWSeq;

int main(int argc, char** argv) {
    std::cout << "Scoring: match=" << MATCH << ", mismatch=" << MISMATCH
              << ", gap=" << GAP << "\n" << std::endl;

    if (argc - 1 == 2) {
        int n = std::stoi(argv[1]);
        int m = std::stoi(argv[2]);

        std::mt19937_64 rnd(42);
        std::string s1 = randomDNA(n, rnd);
        std::string s2 = randomDNA(m, rnd);

        auto start = std::chrono::steady_clock::now();
        auto D = buildMatrix(s1, s2);
        auto fillEnd = std::chrono::steady_clock::now();
        Result r = traceback(s1, s2, D);
        auto end = std::chrono::steady_clock::now();

        double fillMs = std::chrono::duration<double, std::milli>(fillEnd - start).count();
        double totalMs = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << "Lengths: " << n << " x " << m << std::endl;
        std::cout << "Alignment score: " << r.score << std::endl;
        std::printf("Matrix fill time: %.3f ms\n", fillMs);
        std::printf("Total time (fill + traceback): %.3f ms\n", totalMs);
        return 0;
    }

    // Small DNA tests
    runTest("Test 1: textbook example", "GATTACA", "GCATGCT", true);
    runTest("Test 2: identical", "ACGTACGT", "ACGTACGT", false);
    runTest("Test 3: one mismatch", "ACGTACGT", "ACGAACGT", false);
    runTest("Test 4: insertion/deletion", "AGCTAGCT", "AGCTTAGCT", false);
    runTest("Test 5: completely different", "AAAA", "TTTT", false);
    runTest("Test 6: empty vs non-empty", "", "ACGT", false);
    return 0;
}
#endif

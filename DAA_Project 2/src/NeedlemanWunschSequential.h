#pragma once
#include <string>
#include <vector>
#include <random>

// Sequential Needleman-Wunsch global alignment implementation
namespace NWSeq {

constexpr int MATCH = 1;
constexpr int MISMATCH = -1;
constexpr int GAP = -2;

struct Result {
    int score = 0;
    std::string aligned1;
    std::string aligned2;
};

int score(char a, char b);
std::vector<std::vector<int>> buildMatrix(const std::string& s1, const std::string& s2);
Result traceback(const std::string& s1, const std::string& s2,
                 const std::vector<std::vector<int>>& D);
Result align(const std::string& s1, const std::string& s2);
void printMatrix(const std::string& s1, const std::string& s2,
                 const std::vector<std::vector<int>>& D);
std::string matchLine(const std::string& a, const std::string& b);
void runTest(const std::string& name, const std::string& s1, const std::string& s2,
             bool showMatrix);
std::string randomDNA(int len, std::mt19937_64& rnd);

} // namespace NWSeq

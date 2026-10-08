#pragma once
#include <cstdint>
#include <string>
#include <vector>

// Parallel (wavefront / anti-diagonal) Needleman-Wunsch, score only.
// C++ port of NeedlemanWunschParallel.java
namespace NWPar {

// MUST be identical to NeedlemanWunschSequential
constexpr int MATCH = 1;
constexpr int MISMATCH = -1;
constexpr int GAP = -2;

constexpr int MIN_PARALLEL_LEN = 512;
constexpr int REPS = 5;

int sequentialScore(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b);
int parallelScore(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b, int threads);
int wavefrontSingle(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b);
std::vector<uint8_t> randomDNA(int len, uint64_t seed);
std::vector<uint8_t> readSeq(const std::string& path);
double medianMs(std::vector<long long> t);

} // namespace NWPar

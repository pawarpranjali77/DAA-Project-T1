#pragma once
#include <string>

namespace CorrectnessTester {
std::string readSequence(const std::string& fileName);
int getSequentialScore(const std::string& sequenceA, const std::string& sequenceB);
int getParallelScore(const std::string& sequenceA, const std::string& sequenceB, int threads);
}

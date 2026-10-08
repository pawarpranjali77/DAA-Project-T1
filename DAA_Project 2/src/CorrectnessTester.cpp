#include "CorrectnessTester.h"
#include "NeedlemanWunschSequential.h"
#include "NeedlemanWunschParallel.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace CorrectnessTester {

static const int SIZES[] = {100, 500, 1000, 2000, 5000, 10000};

std::string readSequence(const std::string& fileName) {
    std::ifstream in(fileName, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot open file: " + fileName);
    std::stringstream ss;
    ss << in.rdbuf();
    std::string raw = ss.str(), s;
    for (char c : raw)
        if (!std::isspace((unsigned char)c)) s.push_back((char)std::toupper((unsigned char)c));
    return s;
}

int getSequentialScore(const std::string& sequenceA, const std::string& sequenceB) {
    auto matrix = NWSeq::buildMatrix(sequenceA, sequenceB);
    return matrix[sequenceA.length()][sequenceB.length()];
}

int getParallelScore(const std::string& sequenceA, const std::string& sequenceB, int threads) {
    std::vector<uint8_t> a(sequenceA.begin(), sequenceA.end());
    std::vector<uint8_t> b(sequenceB.begin(), sequenceB.end());
    return NWPar::parallelScore(a, b, threads);
}

} // namespace CorrectnessTester

#ifdef CORRECTNESSTESTER_MAIN
using namespace CorrectnessTester;

int main() {
    try {
        std::ofstream writer("results/correctness.csv");
        if (!writer) throw std::runtime_error("Cannot write results/correctness.csv");

        writer << "Size,Sequential Score,Parallel Score,Threads,Match\n";

        std::cout << "==========================================\n";
        std::cout << "      NEEDLEMAN-WUNSCH CORRECTNESS\n";
        std::cout << "==========================================\n";

        for (int size : SIZES) {
            std::string fileA = "datasets/data_" + std::to_string(size) + "_A.txt";
            std::string fileB = "datasets/data_" + std::to_string(size) + "_B.txt";

            std::string sequenceA = readSequence(fileA);
            std::string sequenceB = readSequence(fileB);

            std::cout << "\nDataset size: " << size << std::endl;

            int sequentialScore = getSequentialScore(sequenceA, sequenceB);

            // Test with 1, 2, 4 and 8 threads
            int threadsList[] = {1, 2, 4, 8};
            for (int threads : threadsList) {
                int parallelScore = getParallelScore(sequenceA, sequenceB, threads);
                bool match = (sequentialScore == parallelScore);

                std::cout << "Threads: " << threads << std::endl;
                std::cout << "Sequential Score: " << sequentialScore << std::endl;
                std::cout << "Parallel Score:   " << parallelScore << std::endl;
                std::cout << "Result: " << (match ? "PASS" : "FAIL") << std::endl;

                writer << size << "," << sequentialScore << "," << parallelScore << ","
                       << threads << "," << (match ? "true" : "false") << "\n";
            }
        }
        writer.close();

        std::cout << "\n==========================================\n";
        std::cout << "Correctness testing completed.\n";
        std::cout << "Results saved to results/correctness.csv\n";
        std::cout << "==========================================\n";
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    return 0;
}
#endif

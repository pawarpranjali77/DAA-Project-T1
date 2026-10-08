#include <iostream>
#include "nw.h"
int main(int argc, char** argv) {
    if (argc >= 3) {
        int n = std::stoi(argv[1]), m = std::stoi(argv[2]);
        std::string a = randomDNA(n, 1), b = randomDNA(m, 2);
        int score = 0;
        double ms = timeMs([&] { score = nwScoreSeq(a, b); });
        std::cout << "Sequential " << n << "x" << m << " score=" << score << " time_ms=" << ms << "\n";
        return 0;
    }
    std::vector<std::pair<std::string, std::string>> ex = {
        {"GATTACA", "GCATGCU"}, {"ACGT", "ACGT"}, {"AAAA", "TTTT"}, {"ACGTACGT", "ACGGACT"}};
    for (auto& [a, b] : ex) {
        Alignment r = nwAlign(a, b);
        std::cout << "Score: " << r.score << "\n" << r.a << "\n" << r.b << "\n\n";
    }
}

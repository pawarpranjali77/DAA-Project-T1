#include <iostream>
#include <sstream>
#include "nw.h"
static std::vector<int> parseThreads(const std::string& s) {
    std::vector<int> v; std::stringstream ss(s); std::string x;
    while (std::getline(ss, x, ',')) if (!x.empty()) v.push_back(std::stoi(x));
    return v;
}
int main(int argc, char** argv) {
    std::string a, b; std::vector<int> th = THREADS();
    if (argc >= 4 && std::string(argv[1]) == "-f") {
        a = readSeqFile(argv[2]); b = readSeqFile(argv[3]);
        if (argc >= 5) th = parseThreads(argv[4]);
    } else {
        int n = argc >= 2 ? std::stoi(argv[1]) : 1000;
        if (argc >= 3) th = parseThreads(argv[2]);
        a = randomDNA(n, 1); b = randomDNA(n, 2);
    }
    int seqScore = 0;
    double seqMs = bestMs([&] { seqScore = nwScoreSeq(a, b); });
    std::cout << "length_a,length_b,threads,seq_ms,par_ms,speedup,efficiency,score,match\n";
    for (int t : th) {
        int ps = 0;
        double ms = bestMs([&] { ps = nwScorePar(a, b, t); });
        double sp = seqMs / ms;
        std::cout << a.size() << "," << b.size() << "," << t << "," << seqMs << "," << ms << ","
                  << sp << "," << sp / t << "," << ps << "," << (ps == seqScore) << "\n";
    }
}

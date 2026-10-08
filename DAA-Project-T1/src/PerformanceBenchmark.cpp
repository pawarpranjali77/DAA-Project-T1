#include <iostream>
#include "nw.h"
#include <sys/stat.h>
int main() {
    mkdir("results", 0755);
    std::ofstream out("results/performance.csv");
    out << "length,threads,seq_ms,par_ms,speedup,efficiency\n";
    for (int len : DATA_LENGTHS()) {
        std::string a = readSeqFile(dataPath(len, 'A')), b = readSeqFile(dataPath(len, 'B'));
        double seq = bestMs([&] { nwScoreSeq(a, b); });
        for (int t : THREADS()) {
            double par = bestMs([&] { nwScorePar(a, b, t); });
            double sp = seq / par;
            out << len << "," << t << "," << seq << "," << par << "," << sp << "," << sp / t << "\n";
            std::cout << "len=" << len << " t=" << t << " seq=" << seq << "ms par=" << par << "ms speedup=" << sp << "\n";
        }
    }
    std::ofstream env("results/benchmark_environment.txt");
    env << "hardware_threads: " << std::thread::hardware_concurrency() << "\n"
#ifdef __VERSION__
        << "compiler: " << __VERSION__ << "\n"
#endif
        << "cplusplus: " << __cplusplus << "\n" << "repetitions: best of 3\n";
}

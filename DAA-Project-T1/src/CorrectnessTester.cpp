#include <iostream>
#include "nw.h"
#include <sys/stat.h>
int main() {
    mkdir("results", 0755);
    std::ofstream out("results/correctness.csv");
    out << "length,threads,seq_score,par_score,pass\n";
    int fails = 0;
    for (int len : DATA_LENGTHS()) {
        std::string a = readSeqFile(dataPath(len, 'A')), b = readSeqFile(dataPath(len, 'B'));
        int s = nwScoreSeq(a, b);
        for (int t : THREADS()) {
            int p = nwScorePar(a, b, t);
            bool ok = s == p; fails += !ok;
            out << len << "," << t << "," << s << "," << p << "," << (ok ? "PASS" : "FAIL") << "\n";
            std::cout << "len=" << len << " threads=" << t << " seq=" << s << " par=" << p << (ok ? " PASS" : " FAIL") << "\n";
        }
    }
    std::cout << (fails ? "FAILURES: " + std::to_string(fails) : "All tests passed") << "\n";
    return fails != 0;
}

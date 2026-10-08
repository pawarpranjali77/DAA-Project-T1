#include <iostream>
#include "nw.h"
#include <sys/stat.h>
int main() {
    std::ifstream f("ecoli.fasta");
    if (!f) { std::cerr << "ecoli.fasta not found in project root\n"; return 1; }
    std::string genome, line;
    while (std::getline(f, line)) {
        if (!line.empty() && line[0] == '>') continue;
        for (char c : line) { c = toupper(c); if (c=='A'||c=='T'||c=='C'||c=='G') genome += c; }
    }
    mkdir("datasets", 0755);
    for (int len : DATA_LENGTHS()) {
        if ((int)genome.size() < 500 + len) { std::cerr << "genome too short for " << len << "\n"; return 1; }
        std::ofstream(dataPath(len, 'A')) << genome.substr(0, len) << "\n";
        std::ofstream(dataPath(len, 'B')) << genome.substr(500, len) << "\n";
        std::cout << "wrote length " << len << "\n";
    }
}

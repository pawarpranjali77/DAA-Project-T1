#include <iostream>
#include <sstream>
#include "nw.h"
int main() {
    std::ifstream f("results/performance.csv");
    if (!f) { std::cerr << "run ./cpp_bin/PerformanceBenchmark first\n"; return 1; }
    std::string line; std::getline(f, line);
    struct Row { int len, t; double seq, par, sp, eff; };
    std::vector<Row> rows;
    while (std::getline(f, line)) {
        std::stringstream ss(line); std::string x; std::vector<std::string> c;
        while (std::getline(ss, x, ',')) c.push_back(x);
        if (c.size() < 6) continue;
        rows.push_back({std::stoi(c[0]), std::stoi(c[1]), std::stod(c[2]), std::stod(c[3]), std::stod(c[4]), std::stod(c[5])});
    }
    if (rows.empty()) { std::cerr << "no rows\n"; return 1; }
    Row best = rows[0];
    for (auto& r : rows) { std::cout << r.len << " t=" << r.t << " seq=" << r.seq << " par=" << r.par << " sp=" << r.sp << " eff=" << r.eff << "\n"; if (r.sp > best.sp) best = r; }
    std::cout << "Best speedup: " << best.sp << "x (length " << best.len << ", " << best.t << " threads)\n";
    std::ofstream o("results/performance_summary.csv");
    o << "metric,value\nbest_speedup," << best.sp << "\nbest_length," << best.len << "\nbest_threads," << best.t
      << "\nbest_efficiency," << best.eff << "\nrows," << rows.size() << "\n";
}

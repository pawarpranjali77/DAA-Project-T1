#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct Result {
    int size;
    int threads;
    double seqTime;
    double parTime;
    double speedup;
    double efficiency;
};

static std::vector<Result> readCSV(const std::string& file) {
    std::vector<Result> results;
    std::ifstream br(file);
    if (!br) throw std::runtime_error("Cannot open file: " + file);

    std::string line;
    std::getline(br, line); // skip header

    while (std::getline(br, line)) {
        if (line.empty()) continue;
        std::vector<std::string> p;
        std::stringstream ss(line);
        std::string tok;
        while (std::getline(ss, tok, ',')) p.push_back(tok);

        results.push_back({std::stoi(p[0]), std::stoi(p[1]), std::stod(p[2]),
                           std::stod(p[3]), std::stod(p[4]), std::stod(p[5])});
    }
    return results;
}

static void printAnalysis(const std::vector<Result>& results) {
    std::cout << "===== PERFORMANCE ANALYSIS =====" << std::endl;
    for (const auto& r : results) {
        std::printf("Size: %d | Threads: %d | Sequential: %.3f ms | Parallel: %.3f ms | "
                    "Speedup: %.3f | Efficiency: %.3f\n",
                    r.size, r.threads, r.seqTime, r.parTime, r.speedup, r.efficiency);
    }
}

static void findBestSpeedup(const std::vector<Result>& results) {
    const Result* best = &results.at(0);
    for (const auto& r : results)
        if (r.speedup > best->speedup) best = &r;

    std::cout << std::endl << "===== BEST SPEEDUP =====" << std::endl;
    std::printf("Size: %d | Threads: %d | Speedup: %.3f\n", best->size, best->threads,
                best->speedup);
}

static void saveSummary(const std::vector<Result>& results, const std::string& file) {
    std::ofstream out(file);
    if (!out) throw std::runtime_error("Cannot write " + file);
    out << "size,threads,seq_ms,par_ms,speedup,efficiency\n";
    for (const auto& r : results) {
        char buf[256];
        std::snprintf(buf, sizeof(buf), "%d,%d,%.3f,%.3f,%.3f,%.3f\n", r.size, r.threads,
                      r.seqTime, r.parTime, r.speedup, r.efficiency);
        out << buf;
    }
}

int main() {
    try {
        std::string input = "results/performance.csv";
        auto results = readCSV(input);

        printAnalysis(results);
        findBestSpeedup(results);
        saveSummary(results, "results/performance_summary.csv");

        std::cout << std::endl << "Performance analysis completed." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

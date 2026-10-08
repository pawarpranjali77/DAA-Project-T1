#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static const int SIZES[] = {100, 500, 1000, 2000, 5000, 10000};

static std::string trim(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace((unsigned char)s[b])) b++;
    while (e > b && std::isspace((unsigned char)s[e - 1])) e--;
    return s.substr(b, e - b);
}

static std::string readFASTA(const std::string& fileName) {
    std::ifstream in(fileName);
    if (!in) throw std::runtime_error("Cannot open file: " + fileName);

    std::string sequence, line;
    while (std::getline(in, line)) {
        line = trim(line);

        // Ignore FASTA header
        if (!line.empty() && line[0] == '>') continue;

        // Keep only DNA bases
        for (char ch : line) {
            char c = (char)std::toupper((unsigned char)ch);
            if (c == 'A' || c == 'T' || c == 'C' || c == 'G') sequence.push_back(c);
        }
    }
    return sequence;
}

static void saveSequence(const std::string& fileName, const std::string& sequence) {
    std::ofstream out(fileName, std::ios::binary);
    if (!out) throw std::runtime_error("Cannot write " + fileName);
    out << sequence;
}

static void createDataset(const std::string& sequence, int size,
                          const std::string& outputFolder) {
    if ((int)sequence.length() < size + 500)
        throw std::runtime_error("Not enough DNA sequence for size " + std::to_string(size));

    // Sequence A starts at position 0, sequence B starts at position 500
    std::string sequenceA = sequence.substr(0, size);
    std::string sequenceB = sequence.substr(500, size);

    std::string fileA = outputFolder + "/data_" + std::to_string(size) + "_A.txt";
    std::string fileB = outputFolder + "/data_" + std::to_string(size) + "_B.txt";

    saveSequence(fileA, sequenceA);
    saveSequence(fileB, sequenceB);

    std::cout << "Created size " << size << " -> A: " << fileA << ", B: " << fileB
              << std::endl;
}

int main() {
    std::string fastaFile = "ecoli.fasta";
    std::string outputFolder = "datasets";

    try {
        fs::create_directories(outputFolder);

        std::cout << "Reading real biological DNA dataset..." << std::endl;
        std::string sequence = readFASTA(fastaFile);
        std::cout << "DNA bases extracted: " << sequence.length() << std::endl;

        if (sequence.length() < 10500) {
            std::cout << "Error: FASTA sequence is too short." << std::endl;
            return 0;
        }

        for (int size : SIZES) createDataset(sequence, size, outputFolder);

        std::cout << std::endl << "All datasets generated successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    return 0;
}

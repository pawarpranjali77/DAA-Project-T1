# C++ build for the DAA Needleman-Wunsch project.
# Usage: make        -> builds all executables into cpp_bin/
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -pthread
SRC      := src
BIN      := cpp_bin

TARGETS := NeedlemanWunschSequential NeedlemanWunschParallel NeedlemanWunschOpenMP CorrectnessTester \
           PerformanceBenchmark PerformanceAnalysis RealDatasetProcessor

all: $(addprefix $(BIN)/,$(TARGETS))

$(BIN):
	mkdir -p $(BIN)

$(BIN)/NeedlemanWunschSequential: $(SRC)/NeedlemanWunschSequential.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) -DNEEDLEMANWUNSCHSEQUENTIAL_MAIN $< -o $@

$(BIN)/NeedlemanWunschParallel: $(SRC)/NeedlemanWunschParallel.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) -DNEEDLEMANWUNSCHPARALLEL_MAIN $< -o $@

# OpenMP version of the tiled wavefront (needs -fopenmp; links the helpers from the std::thread file)
$(BIN)/NeedlemanWunschOpenMP: $(SRC)/NeedlemanWunschOpenMP.cpp $(SRC)/NeedlemanWunschParallel.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) -fopenmp -DNEEDLEMANWUNSCHOPENMP_MAIN $^ -o $@

$(BIN)/CorrectnessTester: $(SRC)/CorrectnessTester.cpp $(SRC)/NeedlemanWunschSequential.cpp $(SRC)/NeedlemanWunschParallel.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) -DCORRECTNESSTESTER_MAIN $^ -o $@

$(BIN)/PerformanceBenchmark: $(SRC)/PerformanceBenchmark.cpp $(SRC)/CorrectnessTester.cpp $(SRC)/NeedlemanWunschSequential.cpp $(SRC)/NeedlemanWunschParallel.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BIN)/PerformanceAnalysis: $(SRC)/PerformanceAnalysis.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN)/RealDatasetProcessor: $(SRC)/RealDatasetProcessor.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(BIN)

.PHONY: all clean

CXX ?= g++
CXXFLAGS = -std=c++17 -O2 -Wall -pthread
BIN = cpp_bin
PROGS = NeedlemanWunschSequential NeedlemanWunschParallel RealDatasetProcessor CorrectnessTester PerformanceBenchmark PerformanceAnalysis
all: $(addprefix $(BIN)/,$(PROGS))
$(BIN)/%: src/%.cpp src/nw.h
	@mkdir -p $(BIN) datasets results/graphs
	$(CXX) $(CXXFLAGS) $< -o $@
clean:
	rm -rf $(BIN)
.PHONY: all clean

# Makefile
# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -g --debug -O0 -std=c++20 -Wall -Wextra -Iinclude

# Core library sources (exclude CLI programs)
LIB_SRCS = $(filter-out src/tonemap_cli.cpp, $(wildcard src/*.cpp))
LIB_OBJS = $(LIB_SRCS:src/%.cpp=build/%.o)

# Test executable
TEST_SRCS = $(LIB_SRCS) test/test_cornell_box.cpp
TEST_OBJS = $(TEST_SRCS:src/%.cpp=build/%.o)
TEST_EXEC = build/test

# CLI executable  
CLI_SRCS = $(LIB_SRCS) src/tonemap_cli.cpp
CLI_OBJS = $(CLI_SRCS:src/%.cpp=build/%.o)
CLI_EXEC = build/tonemap

# BMP test executable
BMP_TEST_SRCS = $(LIB_SRCS) test/test_bmp.cpp
BMP_TEST_OBJS = $(BMP_TEST_SRCS:src/%.cpp=build/%.o)
BMP_TEST_EXEC = build/test_bmp

# Default target
all: $(TEST_EXEC) $(CLI_EXEC) $(BMP_TEST_EXEC)

# Build the test executable
$(TEST_EXEC): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build the CLI executable
$(CLI_EXEC): $(CLI_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build the BMP test executable
$(BMP_TEST_EXEC): $(BMP_TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build object files
build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/%.o: test/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean up
clean:
	rm -f build/*.o $(TEST_EXEC) $(CLI_EXEC) $(BMP_TEST_EXEC)

# Run tests
test: $(TEST_EXEC)
	./$(TEST_EXEC)

# Test BMP functionality
test-bmp: $(BMP_TEST_EXEC)
	@echo "Running BMP format tests..."
	./$(BMP_TEST_EXEC)

# Quick test of CLI
test-cli: $(CLI_EXEC)
	@echo "Testing CLI with sample commands..."
	@if [ -f "assets/mpi_office.ppm" ]; then \
		echo "Converting PPM to BMP..."; \
		./$(CLI_EXEC) assets/mpi_office.ppm test_output.bmp convert; \
		echo "Applying gamma correction..."; \
		./$(CLI_EXEC) assets/mpi_office.ppm test_gamma.ppm gamma 2.2; \
		echo "Converting BMP back to PPM..."; \
		./$(CLI_EXEC) test_output.bmp test_roundtrip.ppm convert; \
	else \
		echo "Sample file assets/mpi_office.ppm not found"; \
	fi

.PHONY: all clean test test-bmp test-cli

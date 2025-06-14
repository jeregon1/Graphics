# Makefile
# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -g --debug -O0 -std=c++20 -Wall -Wextra -Iinclude

# Core library sources (exclude CLI programs)
LIB_SRCS = $(filter-out src/tonemap_cli.cpp, $(wildcard src/*.cpp))
LIB_OBJS = $(LIB_SRCS:src/%.cpp=build/%.o)

# Test source files (all test files needed for unified test system)
TEST_SRC_FILES = test/test_main.cpp test/test_p2.cpp test/test_parallel.cpp test/test_cornell_box.cpp test/test_bmp.cpp test/test_geometry.cpp test/test_intersect.cpp
TEST_OBJS = $(LIB_OBJS) $(TEST_SRC_FILES:test/%.cpp=build/test_%.o)
TEST_EXEC = build/test

# CLI executable  
CLI_SRCS = $(LIB_SRCS) src/tonemap_cli.cpp
CLI_OBJS = $(CLI_SRCS:src/%.cpp=build/%.o)
CLI_EXEC = build/tonemap

# Default target
all: $(TEST_EXEC) $(CLI_EXEC)

# Build the unified test executable
$(TEST_EXEC): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build the CLI executable
$(CLI_EXEC): $(CLI_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build object files
build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Build test object files with different naming
build/test_%.o: test/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean up
clean:
	rm -f build/*.o $(TEST_EXEC) $(CLI_EXEC)

# Run tests
test: $(TEST_EXEC)
	./$(TEST_EXEC) all

# Run specific test groups
test-p2: $(TEST_EXEC)
	./$(TEST_EXEC) p2

test-parallel: $(TEST_EXEC)
	./$(TEST_EXEC) parallel

test-cornell: $(TEST_EXEC)
	./$(TEST_EXEC) cornell_box

test-bmp: $(TEST_EXEC)
	./$(TEST_EXEC) bmp

test-geometry: $(TEST_EXEC)
	./$(TEST_EXEC) geometry

test-intersect: $(TEST_EXEC)
	./$(TEST_EXEC) intersect

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

.PHONY: all clean test test-p2 test-parallel test-cornell test-bmp test-geometry test-intersect test-cli

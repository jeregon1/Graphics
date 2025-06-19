# Makefile
# Compiler
CXX = g++
# Compiler flags
CXXFLAGS =  -O2 -std=c++20 -Wall -Wextra -Iinclude

# Core library sources (exclude CLI programs and main)
LIB_SRCS = $(filter-out src/tonemap_cli.cpp src/pathtracer_cli.cpp src/main.cpp, $(wildcard src/*.cpp))
LIB_OBJS = $(LIB_SRCS:src/%.cpp=build/%.o)

# Individual test executables
TEST_EXECUTABLES = build/test_p2 build/test_p3 build/test_bmp build/main build/test_geometry build/test_intersect build/test_parallel build/test_yaml

# CLI executables
TONEMAP_SRCS = $(LIB_SRCS) src/tonemap_cli.cpp
TONEMAP_OBJS = $(TONEMAP_SRCS:src/%.cpp=build/%.o)
TONEMAP_EXEC = build/tonemap

PATHTRACER_SRCS = $(LIB_SRCS) src/pathtracer_cli.cpp
PATHTRACER_OBJS = $(PATHTRACER_SRCS:src/%.cpp=build/%.o)
PATHTRACER_EXEC = build/pathtracer

CLI_EXECS = $(TONEMAP_EXEC) $(PATHTRACER_EXEC)

# Default target
all: $(TEST_EXECUTABLES) $(CLI_EXECS)

# Build individual test executables
build/test_p2: test/test_p2.cpp $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/test_p3: test/test_p3.cpp $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/test_bmp: test/test_bmp.cpp $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/main: src/main.cpp $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/test_geometry: test/test_geometry.cpp $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/test_intersect: test/test_intersect.cpp $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/test_parallel: test/test_parallel.cpp $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/test_yaml: test/test_yaml.cpp $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build the unified test executable
$(TEST_EXEC): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build the CLI executables
$(TONEMAP_EXEC): $(TONEMAP_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(PATHTRACER_EXEC): $(PATHTRACER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build object files
build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean up
clean:
	rm -f build/*.o $(TEST_EXECUTABLES) $(CLI_EXECS)

# Run all tests
test: $(TEST_EXECUTABLES)
	@echo "Running all tests..."
	@for test in $(TEST_EXECUTABLES); do echo "Running $$test..."; ./$$test; echo; done

# Run specific tests
test-p2: build/test_p2
	./build/test_p2

test-p3: build/test_p3
	./build/test_p3

test-parallel: build/test_parallel
	./build/test_parallel

test-cornell: build/test_cornell_box
	./build/test_cornell_box

test-bmp: build/test_bmp
	./build/test_bmp

test-geometry: build/test_geometry
	./build/test_geometry

test-intersect: build/test_intersect
	./build/test_intersect

test-yaml: build/test_yaml
	./build/test_yaml

main: build/main
	./build/main

# Quick test of CLI tools
test-cli: $(CLI_EXECS)
	@echo "Testing tone mapping CLI..."
	@if [ -f "assets/mpi_office.ppm" ]; then \
		echo "Converting PPM to BMP..."; \
		./$(TONEMAP_EXEC) assets/mpi_office.ppm test_output.bmp convert; \
		echo "Applying gamma correction..."; \
		./$(TONEMAP_EXEC) assets/mpi_office.ppm test_gamma.ppm gamma 2.2; \
		echo "Converting BMP back to PPM..."; \
		./$(TONEMAP_EXEC) test_output.bmp test_roundtrip.ppm convert; \
	else \
		echo "Sample file assets/mpi_office.ppm not found"; \
	fi
	@echo "Testing path tracer CLI..."
	@if [ -f "scenes/simple_test.yaml" ]; then \
		echo "Rendering simple test scene..."; \
		./$(PATHTRACER_EXEC) scenes/simple_test.yaml pathtracer_test.ppm 32 256 256; \
	else \
		echo "Sample scene file scenes/simple_test.yaml not found"; \
	fi

# Test path tracer specifically
test-pathtracer: $(PATHTRACER_EXEC)
	@echo "Testing path tracer with reflectance properties..."
	@if [ -f "scenes/cornell_box.yaml" ]; then \
		echo "Rendering Cornell Box with 64 samples..."; \
		./$(PATHTRACER_EXEC) scenes/cornell_box.yaml cornell_pathtracer.ppm 64 512 512; \
	elif [ -f "scenes/simple_test.yaml" ]; then \
		echo "Rendering simple test scene with 32 samples..."; \
		./$(PATHTRACER_EXEC) scenes/simple_test.yaml simple_pathtracer.ppm 32 256 256; \
	else \
		echo "No scene files found in scenes/ directory"; \
	fi

.PHONY: all clean test test-p2 test-p3 test-parallel test-cornell test-bmp test-geometry test-intersect test-yaml test-cli test-pathtracer

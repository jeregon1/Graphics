# Makefile
# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -g --debug -O0 -std=c++20 -Wall -Wextra -Iinclude

# SRCS = $(wildcard src/*.cpp) test/test_intersect.cpp
SRCS = $(wildcard src/*.cpp) test/test_raytracing.cpp

OBJS = $(SRCS:src/%.cpp=build/%.o)

EXEC = build/test

# Default target
all: $(EXEC)

# Build the executable
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build object files
build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean up
clean:
	rm -f build/*.o $(EXEC)

# Run tests
test: $(EXEC)
	./$(EXEC)

.PHONY: all clean test

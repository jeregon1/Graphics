# Makefile
# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -std=c++20 -Wall -Wextra -Iinclude

SRCS = src/geometry.cpp src/Image.cpp src/toneMapping.cpp tests/test.cpp tests/test_geometry.cpp

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

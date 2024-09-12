# p1/Makefile

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -Wextra -I.

# Source files
SRCS = geometry.cpp test_geometry.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable
EXEC = test_geometry

# Default target
all: $(EXEC)

# Build the executable
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean up
clean:
	rm -f $(OBJS) $(EXEC)

# Run tests
test: $(EXEC)
	./$(EXEC)

.PHONY: all clean test
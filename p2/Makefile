# p2/Makefile

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++20 -Wall -Wextra -I.

# Source files
SRCS = Image.cpp toneMapping.cpp test.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable
EXEC = test

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


.PHONY: all clean test
# Nyx Chess Engine Makefile
# Note: This Makefile is designed for Unix-like systems (Linux/macOS)
# For Windows, see README.md for compilation instructions
#
# Compilation: make
# Clean: make clean

CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -Wpedantic
INCLUDES = -Isrc/core -Isrc/search -Isrc/interface
LDFLAGS =

# Source files
SRCS = src/main.cpp \
       src/core/board.cpp \
       src/core/movegen.cpp \
       src/core/eval.cpp \
       src/search/transposition.cpp \
       src/search/search.cpp \
       src/interface/uci.cpp \
       src/interface/cli.cpp

OBJS = $(SRCS:.cpp=.o)

TARGET = nyx

# Test sources
TEST_SRCS = tests/test_movegen.cpp \
            tests/test_eval.cpp \
            tests/test_search.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_TARGET = test_nyx

# Default target
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

# Pattern rule for object files
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

tests/%.o: tests/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Test target
test: $(TEST_TARGET)

$(TEST_TARGET): $(OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

# Clean
clean:
	-del /f /q $(OBJS:src/=%) $(TEST_OBJS:tests/%=%) $(TARGET) $(TEST_TARGET) 2>nul || echo "Cleaned"

# Run tests
run-tests: $(TEST_TARGET)
	./$(TEST_TARGET)

# Help
help:
	@echo "Nyx Chess Engine - Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build the main engine (default)"
	@echo "  test      - Build test executable"
	@echo "  run-tests - Build and run tests"
	@echo "  clean     - Remove all build files"
	@echo "  help      - Show this help"
	@echo ""
	@echo "Usage:"
	@echo "  make              # Build nyx engine"
	@echo "  make test         # Build test suite"
	@echo "  make run-tests    # Build and run tests"
	@echo "  ./nyx uci         # Run in UCI mode"
	@echo "  ./nyx cli         # Run in CLI mode"

.PHONY: all clean test run-tests help

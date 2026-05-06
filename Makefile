# =============================================================================
#  Royal Chess — Makefile
#
#  Targets:
#    make          — build the chess binary (default)
#    make clean    — remove all build artifacts
#    make run      — build and run immediately
#
#  The build uses a separate object directory (build/) so source tree
#  stays clean. Each .cpp gets its own .o; the final link step collects them.
# =============================================================================

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
TARGET   := chess
BUILD    := build
SRC      := src

# Collect every .cpp under src/ recursively
SRCS := $(shell find $(SRC) -name "*.cpp")

# Mirror the source tree under build/ for object files
OBJS := $(patsubst $(SRC)/%.cpp, $(BUILD)/%.o, $(SRCS))

# Default target
.PHONY: all
all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo ""
	@echo "  ♔  Build complete — run with: ./$(TARGET)"
	@echo ""

# Compile each source file, creating subdirectories as needed
$(BUILD)/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(SRC) -c -o $@ $<

# Remove all build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD) $(TARGET)
	@echo "  Cleaned."

# Build and run in one step
.PHONY: run
run: all
	./$(TARGET)
CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
TARGET   := chess
BUILD    := build
SRC      := src

# All .cpp files under src/ plus PipeMode.cpp at the project root
SRCS := $(shell find $(SRC) -name "*.cpp") PipeMode.cpp

OBJS := $(patsubst $(SRC)/%.cpp, $(BUILD)/%.o, $(filter $(SRC)/%, $(SRCS))) \
        $(BUILD)/PipeMode.o

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo ""
	@echo "  ♔  Build complete — run with: ./$(TARGET)"
	@echo ""

$(BUILD)/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(SRC) -I. -c -o $@ $<

# Rule for PipeMode.cpp at the root
$(BUILD)/PipeMode.o: PipeMode.cpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -I$(SRC) -I. -c -o $@ $<

.PHONY: clean
clean:
	rm -rf $(BUILD) $(TARGET)
	@echo "  Cleaned."

.PHONY: run
run: all
	./$(TARGET)
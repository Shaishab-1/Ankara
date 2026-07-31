CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Isrc -Isrc/lexer

BUILD_DIR := build
TARGET    := $(BUILD_DIR)/compiler

SRCS := src/main.cpp
OBJS := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean lexer-test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Lexer testing (Milestone 2) ---
LEXER_L          := src/lexer/lexer.l
LEXER_GEN        := $(BUILD_DIR)/lex.yy.cpp
LEXER_TEST_TARGET:= $(BUILD_DIR)/lexer_test
LEXER_TEST_SRCS  := src/lexer/tokens.cpp src/lexer/main_lexer.cpp

lexer-test: $(LEXER_TEST_TARGET)

$(LEXER_GEN): $(LEXER_L)
	@mkdir -p $(BUILD_DIR)
	flex -o $(LEXER_GEN) $(LEXER_L)

$(LEXER_TEST_TARGET): $(LEXER_GEN) $(LEXER_TEST_SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(LEXER_GEN) $(LEXER_TEST_SRCS)

clean:
	rm -rf $(BUILD_DIR)
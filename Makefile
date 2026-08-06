CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Isrc -Isrc/lexer

BUILD_DIR := build
TARGET    := $(BUILD_DIR)/compiler

SRCS := src/main.cpp
OBJS := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean lexer-test parser-test

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
# --- Parser testing (Milestone 3) ---
PARSER_Y           := src/parser/parser.y
PARSER_TAB_CPP      := $(BUILD_DIR)/parser.tab.cpp
PARSER_TAB_H        := $(BUILD_DIR)/parser.tab.h
PARSER_TEST_TARGET  := $(BUILD_DIR)/parser_test
LEXER_GEN_FOR_PARSER:= $(BUILD_DIR)/lex.yy.for_parser.cpp

.PHONY: parser-test

parser-test: $(PARSER_TEST_TARGET)

$(PARSER_TAB_CPP) $(PARSER_TAB_H): $(PARSER_Y)
	@mkdir -p $(BUILD_DIR)
	bison -d --defines=$(PARSER_TAB_H) -o $(PARSER_TAB_CPP) $(PARSER_Y)

$(LEXER_GEN_FOR_PARSER): $(LEXER_L) $(PARSER_TAB_H)
	@mkdir -p $(BUILD_DIR)
	flex -o $(LEXER_GEN_FOR_PARSER) $(LEXER_L)

$(PARSER_TEST_TARGET): $(PARSER_TAB_CPP) $(LEXER_GEN_FOR_PARSER) src/symbol_table/symbol_table.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(BUILD_DIR) -o $@ $(PARSER_TAB_CPP) $(LEXER_GEN_FOR_PARSER) src/symbol_table/symbol_table.cpp
clean:
	rm -rf $(BUILD_DIR)
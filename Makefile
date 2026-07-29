CC      = gcc
CFLAGS  = -Wall -g \
          -Isrc/ast -Isrc/codegen -Isrc/semantic -Isrc/symbol_table -Ibuild

LEX     = flex
YACC    = bison

BUILD   = build
BIN     = compiler

# Hand-written sources (duplicate semantic-analyzer.c is intentionally excluded)
SRC_C   = src/ast/ast.c \
          src/codegen/codegen.c \
          src/semantic/semantic_analyzer.c \
          src/symbol_table/symbol_table.c

SRC_OBJS = $(SRC_C:.c=.o)
GEN_OBJS = $(BUILD)/lex.yy.o $(BUILD)/parser.tab.o
OBJS     = $(SRC_OBJS) $(GEN_OBJS)

.PHONY: all clean test test-valid test-invalid test-md

all: $(BIN)

$(BUILD):
	mkdir -p $(BUILD)

# Bison: generates parser.tab.c and parser.tab.h
$(BUILD)/parser.tab.c $(BUILD)/parser.tab.h: src/parser/parser.y | $(BUILD)
	$(YACC) -d -o $(BUILD)/parser.tab.c src/parser/parser.y

# Flex: generates lex.yy.c (depends on parser.tab.h for token defs)
$(BUILD)/lex.yy.c: src/lexer/lexer.l $(BUILD)/parser.tab.h
	$(LEX) -o $(BUILD)/lex.yy.c src/lexer/lexer.l

$(BUILD)/lex.yy.o: $(BUILD)/lex.yy.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/parser.tab.o: $(BUILD)/parser.tab.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(BIN)

clean:
	rm -rf $(BUILD) $(BIN)
	rm -f src/ast/*.o src/codegen/*.o src/semantic/*.o src/symbol_table/*.o

# ---- Run the .mc test files directly (already raw source, no markdown fences) ----
test: all test-valid test-invalid

test-valid:
	@echo "==== VALID tests (.mc) ===="
	@for f in tests/valid/*.mc; do \
		echo "--- $$f ---"; \
		./$(BIN) $$f; \
		echo; \
	done

test-invalid:
	@echo "==== INVALID tests (.mc) ===="
	@for f in tests/invalid/*.mc; do \
		echo "--- $$f ---"; \
		./$(BIN) $$f; \
		echo; \
	done

# ---- Run the .md test files by extracting the fenced ```c code block first ----
test-md: all
	@mkdir -p $(BUILD)/tests/valid $(BUILD)/tests/invalid
	@echo "==== VALID tests (.md) ===="
	@for f in tests/valid/*.md; do \
		name=$$(basename $$f .md); \
		sed -n '/```c/,/```/p' $$f | sed '1d;$$d' > $(BUILD)/tests/valid/$$name.mc; \
		echo "--- valid/$$name ---"; \
		./$(BIN) $(BUILD)/tests/valid/$$name.mc; \
		echo; \
	done
	@echo "==== INVALID tests (.md) ===="
	@for f in tests/invalid/*.md; do \
		name=$$(basename $$f .md); \
		sed -n '/```c/,/```/p' $$f | sed '1d;$$d' > $(BUILD)/tests/invalid/$$name.mc; \
		echo "--- invalid/$$name ---"; \
		./$(BIN) $(BUILD)/tests/invalid/$$name.mc; \
		echo; \
	done
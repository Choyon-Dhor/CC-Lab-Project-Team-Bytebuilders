# CC-Lab-Project-Team-Bytebuilders

Compiler Construction Lab Project — Metropolitan University, Bangladesh
Department of CSE

A mini compiler front-end built with Flex and Bison. It takes a small
custom language (int/float/bool, if-else, while, print) and runs it through
lexical analysis, parsing, AST building, semantic checking, and finally
generates Three Address Code (TAC).

## Team — Bytebuilders

| Name | ID | GitHub |
|---|---|---|
| Choyon Dhor (Team Lead) | 231-115-094 | [@Choyon-Dhor](https://github.com/Choyon-Dhor) |
| Anik Dey | 231-115-095 | [@Anikdey095](https://github.com/Anikdey095) |
| Raghu Nandan Roy | 231-115-120 | [@raghu120c](https://github.com/raghu120c) |

Repo: https://github.com/Choyon-Dhor/CC-Lab-Project-Team-Bytebuilders

## What this compiler does

You write a program in our language (a `.mc` file), and the compiler:

1. **Lexer** — turns the raw text into tokens, and prints the token stream
2. **Parser** — checks the tokens against our grammar and builds an AST
3. **Semantic Analyzer** — checks types, scopes, and declarations using a symbol table
4. **Codegen** — walks the AST and prints out Three Address Code

If something's wrong at any stage (bad character, broken syntax, wrong types),
it stops there and tells you what and where.

## The language

```c
int x;
int y;
bool flag;

x = 10;
y = 0;
flag = true;

while (x > 0) {
    y = y + x;
    x = x - 1;
}

if (flag == true) {
    print y;
} else {
    print x;
}
```

Supported stuff:
- Types: `int`, `float`, `bool`
- Declarations, assignments
- Arithmetic (`+ - * / %`), relational (`< > <= >= == !=`), logical (`&& || !`)
- `if`, `if-else`, `while`, `print`
- Nested blocks with their own scope

Not supported (on purpose — outside the project's scope):
functions, arrays, strings, for-loops, switch, type casting.

## Requirements

- Linux (Ubuntu/Debian works fine)
- `gcc`
- `flex`
- `bison`
- `make`

Install everything with:

```bash
sudo apt update
sudo apt install build-essential flex bison
```

## How to build

Clone the repo and just run make from the root:

```bash
git clone https://github.com/Choyon-Dhor/CC-Lab-Project-Team-Bytebuilders.git
cd CC-Lab-Project-Team-Bytebuilders
make
```

This generates the parser/lexer from `src/parser/parser.y` and
`src/lexer/lexer.l`, compiles everything, and produces an executable
called `compiler` in the project root.

To clean up build files:

```bash
make clean
```

## How to run it

```bash
./compiler path/to/your_program.mc
```

Example:

```bash
./compiler tests/valid/sample_valid.mc
```

You'll see four sections printed in order: the token stream, the AST, then
(if everything checks out) the Three Address Code. If there's a lexical,
syntax, or semantic error, it'll print that instead and stop before the
next phase.

### Quick one-line test

```bash
echo 'int x; x = 5; print x;' > /tmp/t.mc && ./compiler /tmp/t.mc
```

## Running the test suite

We've got test programs under `tests/valid/` and `tests/invalid/`, covering
successful compilation plus every error category the lab asks for
(lexical, syntax, redeclaration, undeclared variable, scope violation,
type mismatch, invalid assignment).

```bash
make test       # runs the .mc files directly
make test-md    # extracts and runs the code blocks from the .md test files
```

## Project structure

```
├── docs/            architecture, grammar, and language spec write-ups
├── src/
│   ├── lexer/        lexer.l  (Flex)
│   ├── parser/       parser.y (Bison) + main driver
│   ├── ast/          AST node definitions and printing
│   ├── semantic/     type checking, scope checking
│   └── symbol_table/ symbol table with nested scope support
├── tests/            valid/ and invalid/ test programs
├── examples/         a couple of sample programs
├── Makefile
└── README.md
```

## A few notes on how it's built

- Each block (`if`, `else`, `while`, or a plain `{ }`) opens its own scope.
  A variable declared inside one of these is gone once the block ends —
  we track this with an active/inactive symbol list so we can tell
  "never declared" apart from "declared, but out of scope now."
- The compiler stops at the first phase that finds an error — if there's
  a lexical error, we don't even try to parse; if there's a syntax error,
  we skip semantic checking and codegen. This keeps the error messages
  from getting cluttered with knock-on errors that don't actually matter.
- TAC generation uses the usual temp-variable-and-label approach —
  `t1`, `t2`, ... for intermediate values, `L1`, `L2`, ... for jump targets.

## Team contribution

Work was split roughly along compiler phases (lexer/parser, semantic
analysis + symbol table, codegen + testing), with regular check-ins to
keep everything working together. Commit history reflects individual
contributions from all three members.

## AI usage

We used AI tools (Claude) during development, mainly for debugging help,
writing documentation, and reviewing our code for mistakes we might've
missed. All three of us understand and can explain every part of the
implementation.

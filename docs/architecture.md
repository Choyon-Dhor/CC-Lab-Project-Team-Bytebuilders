# Compiler Architecture

This document describes how the mini compiler front-end is structured and how a
source file flows through each phase, from raw text to Three Address Code (TAC).

## Pipeline Overview

```
source.mc
   │
   ▼
┌─────────────────┐
│  Lexical Analyzer│  src/lexer/lexer.l  (Flex)
│  (Scanner)       │  → stream of tokens
└─────────────────┘
   │
   ▼
┌─────────────────┐
│  Syntax Analyzer │  src/parser/parser.y  (Bison)
│  (Parser)        │  → builds the AST while parsing
└─────────────────┘
   │
   ▼
┌─────────────────┐
│  Abstract Syntax │  src/ast/ast.c, ast.h
│  Tree (AST)      │  → in-memory program representation
└─────────────────┘
   │
   ▼
┌─────────────────┐
│  Semantic        │  src/semantic/semantic_analyzer.c
│  Analyzer        │  → type checking, scope checking, uses Symbol Table
└─────────────────┘
   │            ▲
   │            │ uses
   │      ┌─────────────────┐
   │      │  Symbol Table    │  src/symbol_table/symbol_table.c
   │      │                  │  → nested scopes, declarations
   │      └─────────────────┘
   ▼
┌─────────────────┐
│ Intermediate Code│  src/codegen/codegen.c
│ Generator (TAC)  │  → Three Address Code
└─────────────────┘
   │
   ▼
Three Address Code printed to stdout
```

## Driver (`main`, in `src/parser/parser.y`)

`main()` is the single entry point and enforces a strict fail-fast order between
phases, matching how a real compiler reports the *first* class of error it hits
rather than cascading unrelated errors:

1. Open the source file passed as `argv[1]`.
2. Run `yyparse()`, which pulls tokens from the lexer via `yylex()` and
   incrementally builds the AST (`ast_root`).
3. **Lexical errors gate everything else.** If `lexical_error_count > 0`,
   compilation stops immediately — syntax and semantic phases do not run,
   since the token stream itself was unreliable.
4. **Syntax errors gate semantic/codegen.** If Bison's `yyparse()` returned
   non-zero or `syntax_error_count > 0`, compilation stops before semantic
   analysis.
5. If parsing succeeded, the AST is printed (`ast_print_list`).
6. `semantic_analyze(ast_root)` walks the AST using a `SymbolTable`. If it
   returns a nonzero error count, compilation stops before code generation.
7. Only if all previous phases are clean does `codegen_generate()` run,
   producing a linked list of `TACInstr`, which is printed and then freed.

This ordering means each phase assumes the previous phase's invariants hold —
the semantic analyzer never runs on a malformed AST, and codegen never runs on
a semantically invalid AST.

## Module Responsibilities

### 1. Lexical Analyzer — `src/lexer/lexer.l`
- Implemented with Flex, `%option yylineno` for accurate line numbers in error
  messages.
- Recognizes keywords (`int`, `float`, `bool`, `if`, `else`, `while`, `print`,
  `true`, `false`), identifiers, integer/float literals, all operators, and
  delimiters.
- Strips whitespace, `//` line comments, and `/* ... */` block comments (via
  an exclusive `COMMENT` start condition).
- Any character that doesn't match a rule falls through to the catch-all
  `.` rule, which reports a `Lexical Error` with the offending character and
  line number, increments `lexical_error_count`, and **discards the character**
  rather than returning a token — so the scanner keeps going and can still
  report multiple lexical errors in one pass.

### 2. Syntax Analyzer — `src/parser/parser.y`
- Implemented with Bison, using a `%union` to carry `int`, `float`, `char *`,
  and `ASTNode *` semantic values.
- Precedence/associativity is declared explicitly (`%left OR`, `%left AND`,
  `%precedence NOT`, `%nonassoc` for relational operators, `%left` for
  `+ -` and `* / %`, and `%precedence UMINUS` for unary minus) so the grammar
  is unambiguous for expression parsing.
- Basic error recovery: `stmt: ... | error SEMI { yyerrok; $$ = NULL; }`
  lets the parser resynchronize at the next `;` after a syntax error instead
  of aborting on the first mistake.
- Every grammar action calls into `src/ast/ast.c` factory functions
  (`ast_make_if`, `ast_make_binop`, etc.) to build AST nodes directly during
  the reduce actions — there is no separate "parse tree → AST" conversion
  step.

### 3. Abstract Syntax Tree — `src/ast/ast.h`, `ast.c`
- A single tagged-union-style `struct ASTNode` with an `ASTNodeType` enum
  covering declarations, assignment, if/if-else, while, print, block,
  binary/unary ops, identifiers, and int/float/bool literals.
- Statement sequences are represented as a singly linked list via `next`,
  built with `ast_append_stmt`.
- `ast_print` / `ast_print_list` recursively pretty-print the tree with
  indentation and line numbers, serving as the required AST visualization.

### 4. Symbol Table — `src/symbol_table/symbol_table.c`
- Each `Scope` holds a linked list of `Symbol`s and a pointer to its parent
  scope; `SymbolTable` tracks the current (innermost) scope and a global
  `scope_depth` counter.
- `symtab_enter_scope` / `symtab_exit_scope` push/pop scopes. On exit, symbols
  aren't freed — they're moved to an `inactive_symbols` list, marked
  `active = 0`. This is what lets the semantic analyzer distinguish
  **"never declared"** from **"declared but out of scope"** (see below).
- Lookup has two modes:
  - `symtab_lookup_active` — walks current scope → parent scopes, returns
    only active symbols (correct visibility rules).
  - `symtab_lookup_any` — same walk, but also falls back to
    `inactive_symbols` — used only to produce a more specific
    "scope violation" error instead of a generic "undeclared" error.

### 5. Semantic Analyzer — `src/semantic/semantic_analyzer.c`
- Recursively infers a type string (`"int"`, `"float"`, `"bool"`, or
  `"unknown"` on error) for every expression node via `infer_expr_type`.
- Declarations are inserted into the current scope; a failed insert (name
  already declared in *that* scope) is reported as a redeclaration error.
- Assignments check both that the target is declared/visible, and that the
  inferred expression type `is_assignable` to the target's declared type
  (exact match, or `int → float` widening).
- `if` / `while` conditions must infer to `"bool"`.
- Blocks (`if`, `else`, `while`, bare `{ }`) each open and close their own
  scope via `symtab_enter_scope` / `symtab_exit_scope`, so a variable declared
  inside a block is invisible once that block ends — enforced by the
  active/inactive symbol split described above.
- Errors are accumulated in an `error_count` rather than stopping at the
  first one, so the compiler can report several semantic problems from a
  single run.

### 6. Code Generator — `src/codegen/codegen.c`
- Walks the (already-validated) AST and emits a singly linked list of
  `TACInstr` nodes, each tagged with a `TACOpKind`
  (`COPY`, `BINOP`, `UNOP`, `LABEL`, `GOTO`, `IFFALSE`, `PRINT`).
- Expressions are decomposed into temporaries (`t1`, `t2`, ...) via
  `new_temp`; control flow uses generated labels (`L1`, `L2`, ...) via
  `new_label`.
- `if` (with and without `else`) and `while` are lowered using the classic
  `ifFalse ... goto` / `goto` pattern, matching a textbook TAC scheme.
- `codegen_print` renders the instruction list in a readable textual form;
  `codegen_free_all` releases the list.

## Data Flow Summary

| Phase | Input | Output |
|---|---|---|
| Lexer | Raw source text | Token stream (+ `yylval` for literals/ids) |
| Parser | Token stream | AST (`ASTNode *ast_root`) |
| Semantic Analyzer | AST | Error count (side effect: validated AST, no transformation) |
| Codegen | Validated AST | Linked list of `TACInstr` |

## Known Structural Notes
- The semantic analyzer exists in duplicate as `semantic_analyzer.c`/`.h`
  (underscore, used by the parser) and `semantic-analyzer.c`/`.h` (hyphen,
  currently dead/unused code). Only the underscore version should be built;
  the hyphenated pair is scheduled for removal.
- There is no separate three-address-code-to-assembly backend; TAC generation
  is the final required phase for this lab.

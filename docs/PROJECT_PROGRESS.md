# Project Progress Notes

This document tracks the current status of the Compiler Construction Lab
Project. Last updated after a full code-level review of `src/` against the
project manual's required phases.

## Current Status: All six mandatory phases are implemented

| Phase | Status | Location |
|---|---|---|
| Lexical Analysis | ✅ Implemented | `src/lexer/lexer.l` |
| Syntax Analysis | ✅ Implemented | `src/parser/parser.y` |
| Abstract Syntax Tree | ✅ Implemented | `src/ast/ast.c`, `ast.h` |
| Symbol Table | ✅ Implemented | `src/symbol_table/symbol_table.c` |
| Semantic Analysis | ✅ Implemented | `src/semantic/semantic_analyzer.c` |
| TAC Code Generation | ✅ Implemented | `src/codegen/codegen.c` |
| Test suite (valid + invalid) | ✅ Present | `tests/valid/`, `tests/invalid/` |
| Documentation (`docs/`) | ✅ Filled in | `architecture.md`, `grammar.md`, `language-specification.md` |
| Build automation | ✅ Added | `Makefile` |
| Project Report.pdf | ❌ Not started | — |

> Previous version of this file said "the next development step is to
> implement the lexer and parser" — that was out of date. All core modules
> already existed in `src/` at that point; this file just hadn't been synced
> with the actual codebase.

## Known Issues (found via direct compilation testing)

These were identified by actually compiling the individual source files, not
just reading them:

1. **`src/codegen/codegen.h` — FIXED.** Line 30 had a stray `/` (`/ TACInstr
   *codegen_generate(...)`) instead of a valid declaration or comment, which
   is a hard syntax error the moment the header is included. Corrected to:
   ```c
   TACInstr *codegen_generate(ASTNode *root);
   ```
2. **Duplicate semantic analyzer files — NEEDS CLEANUP.**
   `src/semantic/semantic-analyzer.c` / `.h` (hyphen) are a leftover
   duplicate of `semantic_analyzer.c` / `.h` (underscore, the one actually
   used by `parser.y`). Both define `semantic_analyze()`, which causes a
   linker error (`multiple definition of 'semantic_analyze'`) if both get
   compiled into the same build. **Action: delete the hyphenated pair.**
3. **Build automation** — a `Makefile` has been written (builds via
   Flex/Bison + gcc, with `make test` / `make test-md` targets to run the
   files under `tests/`). Verify it still matches the final directory layout
   once item 2 above is cleaned up.

## Team Goal
Build a mini compiler front-end using Flex, Bison, and C/C++.
(Status: core goal achieved — pending the cleanup items above and the
written Project Report.)

## Remaining Work
- [ ] Delete `src/semantic/semantic-analyzer.c` and `semantic-analyzer.h`
- [ ] Confirm `make` builds cleanly end-to-end with Flex/Bison installed
- [ ] Run `make test` / `make test-md` against the full `tests/` suite and
      confirm actual output matches each test's "Expected" section
- [ ] Write `Project Report.pdf`
- [ ] Final pass on commit history / repository organization before
      submission

## Test Coverage Snapshot

Valid programs (`tests/valid/`): declarations, assignment, arithmetic,
operator precedence, if/else, while, a full combined program.

Invalid programs (`tests/invalid/`), one per required error category:
lexical error, syntax error, redeclaration, undeclared variable, scope
violation, type mismatch, invalid assignment.

This matches the manual's requirement to demonstrate lexical, syntactic, and
semantic error handling with concrete examples.

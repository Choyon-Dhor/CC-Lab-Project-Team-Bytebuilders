# Language Specification

This document specifies the mini programming language implemented by this
compiler front-end, as defined by the actual lexer (`src/lexer/lexer.l`),
grammar (`src/parser/parser.y`), and semantic analyzer
(`src/semantic/semantic_analyzer.c`).

## 1. Types

Three primitive types are supported:

|       Type     | Keyword | Example literal |
|----------------|---------|-----------------|
|       Integer  | `int`   | `10`, `0`, `42` |
| Floating point | `float` | `3.14`, `0.5`   |
|    Boolean     | `bool`  | `true`, `false` |

There is no `string`, array, function, or struct type — this is intentionally
a minimal expression/statement language for the lab's scope.

## 2. Lexical Elements

### Identifiers
```
{LETTER}({LETTER}|{DIGIT})*      LETTER = [a-zA-Z_]   DIGIT = [0-9]
```
Examples: `x`, `count`, `_temp`, `value2`.

### Literals
- **Integer literal**: `{DIGIT}+`  → e.g. `0`, `123`
- **Float literal**: `{DIGIT}+"."{DIGIT}+`  → e.g. `3.14`, `0.5`
  (a bare `.5` or `5.` is **not** matched by this rule)
- **Boolean literal**: the keywords `true` / `false`

### Keywords (reserved, cannot be used as identifiers)
```
int  float  bool  if  else  while  print  true  false
```

### Operators

| Category | Operators |
|-----------|-------------------------|
| Arithmetic | `+`  `-`  `*`  `/`  `%` |
| Relational | `<`  `>`  `<=`  `>=`  `==`  `!=` |
| Logical | `&&`  `\|\|`  `!` |
| Assignment | `=` |

### Delimiters
```
{  }  (  )  ;
```

### Comments (discarded by the lexer, not part of the grammar)
```
// single-line comment
/* multi-line
   comment */
```

### Whitespace
Spaces, tabs, carriage returns, and newlines are discarded (newlines still
advance `yylineno` for accurate error reporting).

### Invalid tokens
Any character not matched by the rules above (e.g. `@`, `#`, `$`) is reported
as a **Lexical Error** with its line number, and the compiler stops before
syntax/semantic analysis if any lexical error occurred.

## 3. Program Structure

A program is a sequence of zero or more statements:

```
program      → stmt_list
stmt_list    → ε | stmt_list stmt
```

## 4. Statements

### 4.1 Declaration
```
type IDENTIFIER ;
```
Example:
```c
int x;
float pi;
bool flag;
```
- Declaring the same identifier twice **in the same scope** is a semantic
  error (redeclaration).
- Declaring a variable does not initialize it, and does not implicitly assign
  a default value at the language level (the value is whatever
  the generated TAC's copy semantics leave it as — declarations do not emit
  any TAC instruction on their own).

### 4.2 Assignment
```
IDENTIFIER = expr ;
```
Example:
```c
x = 10;
y = x + 3;
```
- The identifier must already be declared and visible in the current or an
  enclosing scope.
- The expression's inferred type must be **assignable** to the variable's
  declared type:
  - Exact type match (`int = int`, `float = float`, `bool = bool`), or
  - `int → float` widening (assigning an `int` expression to a `float`
    variable is allowed).
  - Any other combination (e.g. assigning `bool` to `int`, or `int/float` to
    `bool`) is a semantic type-mismatch error.

### 4.3 If / If-Else
```
if ( expr ) block
if ( expr ) block else block
```
Example:
```c
if (x > 0) {
    print x;
} else {
    print 0;
}
```
- The condition expression must infer to type `bool`.
- Each branch introduces its own nested scope — variables declared inside a
  branch are not visible outside it (see §6, Scoping).

### 4.4 While
```
while ( expr ) block
```
Example:
```c
while (i > 0) {
    print i;
    i = i - 1;
}
```
- The condition must infer to type `bool`.
- The loop body introduces its own nested scope.

### 4.5 Print
```
print expr ;
```
Example:
```c
print x + 1;
```
- `print` accepts any expression whose type can be inferred (no type
  restriction beyond that — printing a `bool` or `float` is legal).

### 4.6 Block
```
{ stmt_list }
```
A bare block (not attached to `if`/`while`) is legal and introduces its own
scope, e.g.:
```c
{
    int local;
    local = 1;
}
```

### 4.7 Error-recovery statement
If a statement contains a syntax error, the parser discards tokens up to the
next `;` and continues parsing subsequent statements, rather than aborting on
the first syntax error.

## 5. Expressions

Expression grammar, from lowest to highest precedence (matching the
`%left` / `%precedence` / `%nonassoc` declarations in `parser.y`):

```
expr → expr || expr
     | expr && expr
     | ! expr
     | expr < expr | expr > expr | expr <= expr | expr >= expr
     | expr == expr | expr != expr
     | expr + expr | expr - expr
     | expr * expr | expr / expr | expr % expr
     | - expr                      (unary minus)
     | ( expr )
     | IDENTIFIER
     | INT_LIT
     | FLOAT_LIT
     | true | false
```

Precedence (loosest to tightest binding):
```
||                     (left-assoc)
&&                     (left-assoc)
!                      (unary, non-assoc precedence level)
< > <= >= == !=         (non-associative — cannot chain, e.g. a < b < c is invalid)
+ -                    (left-assoc)
* / %                  (left-assoc)
unary -                (highest, via %prec UMINUS)
```

### 5.1 Type rules for operators

| Operator(s) | Required operand types | Result type |
|---|---|---|
| `+ - * /  %` | both numeric (`int` or `float`) | `float` if either operand is `float`, else `int` |
| `< > <= >=` | both numeric | `bool` |
| `== !=` | both numeric, **or** both `bool` | `bool` |
| `&& \|\|` | both `bool` | `bool` |
| unary `!` | `bool` | `bool` |
| unary `-` | numeric | same as operand |

Any operand type combination not listed above is a semantic
"invalid expression" error (e.g. `true + 1`, `1 && 2`, `x < true`).

## 6. Scoping Rules

- The program has one implicit **global scope**.
- `if`/`else` branches, `while` bodies, and bare `{ }` blocks each introduce a
  **new nested scope**.
- A variable is visible in the scope it was declared in and any scope nested
  inside it, but **not** after that scope has ended.
- Referencing a variable that was declared in a scope that has already ended
  is reported as a **scope violation** (distinct from "undeclared" — the
  compiler tracks previously-declared-but-now-inactive symbols specifically
  to give this more precise diagnostic).
- Redeclaring the same name **within the same scope** is a semantic error;
  the same name may be reused in a different (e.g. nested) scope — this
  is standard shadowing and is not flagged as an error.

## 7. Error Categories

| Category | Example | Detected by |
|---|---|---|
| Lexical Error | invalid character `@` | Lexer |
| Syntax Error | missing `;`, unmatched `(` | Parser |
| Semantic Error — Redeclaration | `int x; int x;` in same scope | Semantic analyzer |
| Semantic Error — Undeclared variable | using `y` that was never declared | Semantic analyzer |
| Semantic Error — Scope violation | using a variable after its block ended | Semantic analyzer |
| Semantic Error — Type mismatch | `bool_var = 10 + 5;` | Semantic analyzer |
| Semantic Error — Invalid expression | `true + 1`, `1 && 2` | Semantic analyzer |

Compilation stops after the first phase that produces one or more errors —
lexical errors block syntax/semantic analysis, syntax errors block semantic
analysis and codegen, and semantic errors block codegen.

## 8. What Is Explicitly Out of Scope

To keep the language's scope aligned with the lab requirements, the following
are **not** supported (and are not planned unless the manual is updated):
- Functions / procedures
- Arrays or any composite/collection types
- Strings
- `for` loops, `switch`, `break`/`continue`
- Type casting syntax (only the implicit `int → float` widening rule exists)
- Multiple declarations in one statement (`int x, y;`)

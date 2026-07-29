# Context-Free Grammar

This is the CFG actually implemented in `src/parser/parser.y` (Bison),
transcribed from the `%%` rules section, with the semantic (AST-building)
actions omitted for readability. See `parser.y` itself for the exact
`ast_make_*` calls tied to each production.

## Start Symbol
```
%start program
```

## Grammar

```bnf
program
    : stmt_list

stmt_list
    : /* empty */
    | stmt_list stmt

stmt
    : decl_stmt
    | assign_stmt
    | if_stmt
    | while_stmt
    | print_stmt
    | block
    | error SEMI                       /* syntax-error recovery point */

type
    : INT
    | FLOAT
    | BOOL

decl_stmt
    : type IDENTIFIER SEMI

assign_stmt
    : IDENTIFIER ASSIGN expr SEMI

if_stmt
    : IF LPAREN expr RPAREN block
    | IF LPAREN expr RPAREN block ELSE block

while_stmt
    : WHILE LPAREN expr RPAREN block

print_stmt
    : PRINT expr SEMI

block
    : LBRACE stmt_list RBRACE

expr
    : expr OR expr
    | expr AND expr
    | NOT expr
    | expr LT expr
    | expr GT expr
    | expr LE expr
    | expr GE expr
    | expr EQ expr
    | expr NE expr
    | expr PLUS expr
    | expr MINUS expr
    | expr MUL expr
    | expr DIV expr
    | expr MOD expr
    | MINUS expr                       %prec UMINUS
    | LPAREN expr RPAREN
    | IDENTIFIER
    | INT_LIT
    | FLOAT_LIT
    | TRUE
    | FALSE
```

## Terminals (Tokens)

Produced by `src/lexer/lexer.l`:

```
Keywords:     INT  FLOAT  BOOL  IF  ELSE  WHILE  PRINT  TRUE  FALSE
Literals:     IDENTIFIER  INT_LIT  FLOAT_LIT
Arithmetic:   PLUS  MINUS  MUL  DIV  MOD
Relational:   LT  GT  LE  GE  EQ  NE
Logical:      AND  OR  NOT
Assignment:   ASSIGN
Delimiters:   LBRACE  RBRACE  LPAREN  RPAREN  SEMI
```

## Precedence and Associativity Declarations

Declared in `parser.y`, lowest to highest precedence:

```
%left OR
%left AND
%precedence NOT
%nonassoc LT GT LE GE EQ NE
%left PLUS MINUS
%left MUL DIV MOD
%precedence UMINUS
```

This resolves standard ambiguities without needing extra grammar rules:
- `a + b * c` parses as `a + (b * c)` (MUL/DIV/MOD bind tighter than PLUS/MINUS).
- `a && b || c` parses as `(a && b) || c` (AND binds tighter than OR).
- `-a + b` parses as `(-a) + b` (UMINUS binds tightest).
- `a < b < c` is a **grammar-level syntax error**, since relational operators
  are declared `%nonassoc` (deliberately disallowing chained comparisons,
  since the result of a relational expression is `bool`, not a value that can
  meaningfully be re-compared with `<`/`>` in this language).

## Known Ambiguity: Dangling Else

The two `if_stmt` alternatives:
```
IF LPAREN expr RPAREN block
IF LPAREN expr RPAREN block ELSE block
```
form the classic "dangling else" ambiguity when `if`s are nested without
braces disambiguating them. Bison resolves this via its default shift
preference (favoring shifting the `ELSE` token rather than reducing the
inner `if` early), which yields the conventional behavior of an `else`
attaching to the **nearest unmatched `if`**. This produces one expected
shift/reduce conflict when building with `bison -Wall`, which is normal
and does not need a grammar rewrite for this project's scope.

## Error-Recovery Production

```
stmt : error SEMI { yyerrok; $$ = NULL; }
```
When a statement fails to parse, Bison's error-recovery mechanism discards
tokens until it can shift a `SEMI`, then resumes normal parsing from the next
statement. This is what allows the compiler to continue past one broken
statement instead of aborting the whole parse on the first syntax error
(see `tests/invalid/syntax_error.md` for an example).

## AST Construction

Every non-trivial production calls a factory function from `src/ast/ast.h` to
build an `ASTNode` as part of the reduce action, e.g.:

| Production | AST factory call |
|---|---|
| `decl_stmt` | `ast_make_decl(type, name, line)` |
| `assign_stmt` | `ast_make_assign(name, expr, line)` |
| `if_stmt` | `ast_make_if(cond, then_block, else_block_or_NULL, line)` |
| `while_stmt` | `ast_make_while(cond, body, line)` |
| `print_stmt` | `ast_make_print(expr, line)` |
| `block` | `ast_make_block(stmt_list, line)` |
| binary `expr` rules | `ast_make_binop(op_string, left, right, line)` |
| `NOT expr` / unary `MINUS expr` | `ast_make_unop(op_string, operand, line)` |
| `IDENTIFIER` (in expr) | `ast_make_identifier(name, line)` |
| `INT_LIT` / `FLOAT_LIT` / `TRUE` / `FALSE` | `ast_make_int_lit` / `ast_make_float_lit` / `ast_make_bool_lit` |

There is no intermediate concrete parse tree — the AST is built directly
during parsing, one reduce action at a time.

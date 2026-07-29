#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

typedef enum
{
    TAC_COPY,
    TAC_BINOP,
    TAC_UNOP,
    TAC_LABEL,
    TAC_GOTO,
    TAC_IFFALSE,
    TAC_PRINT
} TACOpKind;

typedef struct TACInstr
{
    TACOpKind kind;

    char *result;
    char *arg1;
    char *op;
    char *arg2;
    char *label;

    struct TACInstr *next;
} TACInstr;

TACInstr *codegen_generate(ASTNode *root);

void codegen_print(TACInstr *list);

void codegen_free_all(TACInstr *list);

#endif

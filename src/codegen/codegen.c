#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"

static TACInstr *head = NULL;
static TACInstr *tail = NULL;

static int temp_count = 0;
static int label_count = 0;

static char *xstrdup(const char *s)
{
    if (!s)
        return NULL;
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (!copy)
    {
        fprintf(stderr, "Fatal: out of memory in codegen\n");
        exit(1);
    }
    memcpy(copy, s, len);
    return copy;
}

static char *new_temp(void)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "t%d", ++temp_count);
    return xstrdup(buf);
}

static char *new_label(void)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "L%d", ++label_count);
    return xstrdup(buf);
}

static TACInstr *new_instr(TACOpKind kind)
{
    TACInstr *ins = calloc(1, sizeof(TACInstr));
    if (!ins)
    {
        fprintf(stderr, "Fatal: out of memory in codegen\n");
        exit(1);
    }
    ins->kind = kind;
    return ins;
}

static void emit(TACInstr *ins)
{
    if (!head)
    {
        head = tail = ins;
    }
    else
    {
        tail->next = ins;
        tail = ins;
    }
}

static void emit_copy(const char *result, const char *arg1)
{
    TACInstr *ins = new_instr(TAC_COPY);
    ins->result = xstrdup(result);
    ins->arg1 = xstrdup(arg1);
    emit(ins);
}

static void emit_binop(const char *result, const char *arg1, const char *op, const char *arg2)
{
    TACInstr *ins = new_instr(TAC_BINOP);
    ins->result = xstrdup(result);
    ins->arg1 = xstrdup(arg1);
    ins->op = xstrdup(op);
    ins->arg2 = xstrdup(arg2);
    emit(ins);
}

static void emit_unop(const char *result, const char *op, const char *arg1)
{
    TACInstr *ins = new_instr(TAC_UNOP);
    ins->result = xstrdup(result);
    ins->op = xstrdup(op);
    ins->arg1 = xstrdup(arg1);
    emit(ins);
}

static void emit_label(const char *label)
{
    TACInstr *ins = new_instr(TAC_LABEL);
    ins->label = xstrdup(label);
    emit(ins);
}

static void emit_goto(const char *label)
{
    TACInstr *ins = new_instr(TAC_GOTO);
    ins->label = xstrdup(label);
    emit(ins);
}

static void emit_iffalse(const char *arg1, const char *label)
{
    TACInstr *ins = new_instr(TAC_IFFALSE);
    ins->arg1 = xstrdup(arg1);
    ins->label = xstrdup(label);
    emit(ins);
}

static void emit_print(const char *arg1)
{
    TACInstr *ins = new_instr(TAC_PRINT);
    ins->arg1 = xstrdup(arg1);
    emit(ins);
}

static char *gen_expr(ASTNode *node)
{
    if (!node)
        return xstrdup("0");

    char buf[64];

    switch (node->type)
    {
    case NODE_IDENTIFIER:
        return xstrdup(node->name);

    case NODE_INT_LIT:
        snprintf(buf, sizeof(buf), "%d", node->int_val);
        return xstrdup(buf);

    case NODE_FLOAT_LIT:
        snprintf(buf, sizeof(buf), "%g", node->float_val);
        return xstrdup(buf);

    case NODE_BOOL_LIT:
        return xstrdup(node->bool_val ? "true" : "false");

    case NODE_UNOP:
    {
        char *operand = gen_expr(node->left);
        char *result = new_temp();
        emit_unop(result, node->op, operand);
        free(operand);
        return result;
    }

    case NODE_BINOP:
    {
        char *left = gen_expr(node->left);
        char *right = gen_expr(node->right);
        char *result = new_temp();
        emit_binop(result, left, node->op, right);
        free(left);
        free(right);
        return result;
    }

    default:
        /* Should not happen for well-formed expression subtrees */
        return xstrdup("0");
    }
}

static void gen_stmt_list(ASTNode *list);

static void gen_stmt(ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case NODE_DECL:

        break;

    case NODE_ASSIGN:
    {
        char *place = gen_expr(node->assign_expr);
        emit_copy(node->assign_name, place);
        free(place);
        break;
    }

    case NODE_IF:
    {
        char *cond = gen_expr(node->if_cond);

        if (node->if_else)
        {
            char *else_label = new_label();
            char *end_label = new_label();

            emit_iffalse(cond, else_label);
            free(cond);

            gen_stmt_list(node->if_then->block_stmts);
            emit_goto(end_label);

            emit_label(else_label);
            gen_stmt_list(node->if_else->block_stmts);

            emit_label(end_label);

            free(else_label);
            free(end_label);
        }
        else
        {
            char *end_label = new_label();

            emit_iffalse(cond, end_label);
            free(cond);

            gen_stmt_list(node->if_then->block_stmts);

            emit_label(end_label);
            free(end_label);
        }
        break;
    }

    case NODE_WHILE:
    {
        char *start_label = new_label();
        char *end_label = new_label();

        emit_label(start_label);
        char *cond = gen_expr(node->while_cond);
        emit_iffalse(cond, end_label);
        free(cond);

        gen_stmt_list(node->while_body->block_stmts);
        emit_goto(start_label);

        emit_label(end_label);

        free(start_label);
        free(end_label);
        break;
    }

    case NODE_PRINT:
    {
        char *place = gen_expr(node->print_expr);
        emit_print(place);
        free(place);
        break;
    }

    case NODE_BLOCK:
        gen_stmt_list(node->block_stmts);
        break;

    default:
        break;
    }
}

static void gen_stmt_list(ASTNode *list)
{
    for (ASTNode *cur = list; cur != NULL; cur = cur->next)
    {
        gen_stmt(cur);
    }
}

TACInstr *codegen_generate(ASTNode *root)
{
    head = tail = NULL;
    temp_count = label_count = 0;

    gen_stmt_list(root);

    return head;
}

void codegen_print(TACInstr *list)
{
    for (TACInstr *ins = list; ins != NULL; ins = ins->next)
    {
        switch (ins->kind)
        {
        case TAC_COPY:
            printf("%s = %s\n", ins->result, ins->arg1);
            break;
        case TAC_BINOP:
            printf("%s = %s %s %s\n", ins->result, ins->arg1, ins->op, ins->arg2);
            break;
        case TAC_UNOP:
            printf("%s = %s%s\n", ins->result, ins->op, ins->arg1);
            break;
        case TAC_LABEL:
            printf("%s:\n", ins->label);
            break;
        case TAC_GOTO:
            printf("goto %s\n", ins->label);
            break;
        case TAC_IFFALSE:
            printf("ifFalse %s goto %s\n", ins->arg1, ins->label);
            break;
        case TAC_PRINT:
            printf("print %s\n", ins->arg1);
            break;
        }
    }
}

void codegen_free_all(TACInstr *list)
{
    while (list)
    {
        TACInstr *next = list->next;
        free(list->result);
        free(list->arg1);
        free(list->op);
        free(list->arg2);
        free(list->label);
        free(list);
        list = next;
    }
}

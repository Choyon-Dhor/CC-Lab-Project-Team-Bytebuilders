#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

static ASTNode *new_node(ASTNodeType type, int line) {
    ASTNode *n = calloc(1, sizeof(ASTNode));
    if (!n) {
        fprintf(stderr, "Fatal: out of memory while creating AST node\n");
        exit(1);
    }
    n->type = type;
    n->line = line;
    return n;
}

ASTNode *ast_make_decl(const char *type, const char *name, int line) {
    ASTNode *n = new_node(NODE_DECL, line);
    n->decl_type = strdup(type);
    n->decl_name = strdup(name);
    return n;
}

ASTNode *ast_make_assign(const char *name, ASTNode *expr, int line) {
    ASTNode *n = new_node(NODE_ASSIGN, line);
    n->assign_name = strdup(name);
    n->assign_expr = expr;
    return n;
}

ASTNode *ast_make_if(ASTNode *cond, ASTNode *then_b, ASTNode *else_b, int line) {
    ASTNode *n = new_node(NODE_IF, line);
    n->if_cond = cond;
    n->if_then = then_b;
    n->if_else = else_b;
    return n;
}

ASTNode *ast_make_while(ASTNode *cond, ASTNode *body, int line) {
    ASTNode *n = new_node(NODE_WHILE, line);
    n->while_cond = cond;
    n->while_body = body;
    return n;
}

ASTNode *ast_make_print(ASTNode *expr, int line) {
    ASTNode *n = new_node(NODE_PRINT, line);
    n->print_expr = expr;
    return n;
}

ASTNode *ast_make_block(ASTNode *stmts, int line) {
    ASTNode *n = new_node(NODE_BLOCK, line);
    n->block_stmts = stmts;
    return n;
}

ASTNode *ast_make_binop(const char *op, ASTNode *l, ASTNode *r, int line) {
    ASTNode *n = new_node(NODE_BINOP, line);
    n->op = strdup(op);
    n->left = l;
    n->right = r;
    return n;
}

ASTNode *ast_make_unop(const char *op, ASTNode *operand, int line) {
    ASTNode *n = new_node(NODE_UNOP, line);
    n->op = strdup(op);
    n->left = operand;
    return n;
}

ASTNode *ast_make_identifier(const char *name, int line) {
    ASTNode *n = new_node(NODE_IDENTIFIER, line);
    n->name = strdup(name);
    return n;
}

ASTNode *ast_make_int_lit(int val, int line) {
    ASTNode *n = new_node(NODE_INT_LIT, line);
    n->int_val = val;
    return n;
}

ASTNode *ast_make_float_lit(float val, int line) {
    ASTNode *n = new_node(NODE_FLOAT_LIT, line);
    n->float_val = val;
    return n;
}

ASTNode *ast_make_bool_lit(int val, int line) {
    ASTNode *n = new_node(NODE_BOOL_LIT, line);
    n->bool_val = val;
    return n;
}

ASTNode *ast_append_stmt(ASTNode *list, ASTNode *stmt) {
    if (!list) return stmt;
    ASTNode *cur = list;
    while (cur->next) cur = cur->next;
    cur->next = stmt;
    return list;
}


static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
}

void ast_print(ASTNode *node, int indent) {
    if (!node) return;
    print_indent(indent);

    switch (node->type) {
        case NODE_DECL:
            printf("Decl (%s %s)  [line %d]\n", node->decl_type, node->decl_name, node->line);
            break;

        case NODE_ASSIGN:
            printf("Assign %s =  [line %d]\n", node->assign_name, node->line);
            ast_print(node->assign_expr, indent + 1);
            break;

        case NODE_IF:
            printf("If  [line %d]\n", node->line);
            print_indent(indent + 1); printf("Cond:\n");
            ast_print(node->if_cond, indent + 2);
            print_indent(indent + 1); printf("Then:\n");
            ast_print(node->if_then, indent + 2);
            if (node->if_else) {
                print_indent(indent + 1); printf("Else:\n");
                ast_print(node->if_else, indent + 2);
            }
            break;

        case NODE_WHILE:
            printf("While  [line %d]\n", node->line);
            print_indent(indent + 1); printf("Cond:\n");
            ast_print(node->while_cond, indent + 2);
            print_indent(indent + 1); printf("Body:\n");
            ast_print(node->while_body, indent + 2);
            break;

        case NODE_PRINT:
            printf("Print  [line %d]\n", node->line);
            ast_print(node->print_expr, indent + 1);
            break;

        case NODE_BLOCK:
            printf("Block  [line %d]\n", node->line);
            ast_print_list(node->block_stmts, indent + 1);
            break;

        case NODE_BINOP:
            printf("BinOp '%s'  [line %d]\n", node->op, node->line);
            ast_print(node->left, indent + 1);
            ast_print(node->right, indent + 1);
            break;

        case NODE_UNOP:
            printf("UnOp '%s'  [line %d]\n", node->op, node->line);
            ast_print(node->left, indent + 1);
            break;

        case NODE_IDENTIFIER:
            printf("Identifier '%s'  [line %d]\n", node->name, node->line);
            break;

        case NODE_INT_LIT:
            printf("IntLit %d  [line %d]\n", node->int_val, node->line);
            break;

        case NODE_FLOAT_LIT:
            printf("FloatLit %g  [line %d]\n", node->float_val, node->line);
            break;

        case NODE_BOOL_LIT:
            printf("BoolLit %s  [line %d]\n", node->bool_val ? "true" : "false", node->line);
            break;

        default:
            printf("UnknownNode  [line %d]\n", node->line);
    }
}

void ast_print_list(ASTNode *list, int indent) {
    for (ASTNode *cur = list; cur != NULL; cur = cur->next) {
        ast_print(cur, indent);
    }
}

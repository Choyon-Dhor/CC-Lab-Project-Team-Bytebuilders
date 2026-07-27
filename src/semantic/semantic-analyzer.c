#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "semantic_analyzer.h"
#include "symbol_table.h"

static void semantic_error(int line, const char *message)
{
    fprintf(stderr, "Semantic Error at line %d: %s\n", line, message);
}

static int is_numeric(const char *type)
{
    return strcmp(type, "int") == 0 || strcmp(type, "float") == 0;
}

static int is_assignable(const char *target_type, const char *expr_type)
{
    if (!target_type || !expr_type)
        return 0;
    if (strcmp(target_type, expr_type) == 0)
        return 1;
    if (strcmp(target_type, "float") == 0 && strcmp(expr_type, "int") == 0)
        return 1;
    return 0;
}

static const char *infer_expr_type(ASTNode *node, SymbolTable *table, int *error_count)
{
    if (!node)
        return "unknown";

    switch (node->type)
    {
    case NODE_IDENTIFIER:
    {
        Symbol *sym = symtab_lookup_active(table, node->name);
        if (sym)
            return sym->type;

        Symbol *inactive = symtab_lookup_any(table, node->name);
        if (inactive)
        {
            semantic_error(node->line, "scope violation: variable is not visible outside its block");
            (*error_count)++;
        }
        else
        {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "undeclared variable '%s'", node->name);
            semantic_error(node->line, buffer);
            (*error_count)++;
        }
        return "unknown";
    }

    case NODE_INT_LIT:
        return "int";

    case NODE_FLOAT_LIT:
        return "float";

    case NODE_BOOL_LIT:
        return "bool";

    case NODE_UNOP:
    {
        const char *operand_type = infer_expr_type(node->left, table, error_count);
        if (strcmp(node->op, "-") == 0)
        {
            if (is_numeric(operand_type))
                return operand_type;
            semantic_error(node->line, "invalid expression: unary '-' requires a numeric operand");
            (*error_count)++;
            return "unknown";
        }
        if (strcmp(node->op, "!") == 0)
        {
            if (strcmp(operand_type, "bool") == 0)
                return "bool";
            semantic_error(node->line, "invalid expression: unary '!' requires a boolean operand");
            (*error_count)++;
            return "unknown";
        }
        return "unknown";
    }

    case NODE_BINOP:
    {
        const char *left_type = infer_expr_type(node->left, table, error_count);
        const char *right_type = infer_expr_type(node->right, table, error_count);

        if (strcmp(left_type, "unknown") == 0 || strcmp(right_type, "unknown") == 0)
        {
            return "unknown";
        }

        if (strcmp(node->op, "&&") == 0 || strcmp(node->op, "||") == 0)
        {
            if (strcmp(left_type, "bool") == 0 && strcmp(right_type, "bool") == 0)
            {
                return "bool";
            }
            semantic_error(node->line, "invalid expression: logical operators require boolean operands");
            (*error_count)++;
            return "unknown";
        }

        if (strcmp(node->op, "+") == 0 || strcmp(node->op, "-") == 0 || strcmp(node->op, "*") == 0 || strcmp(node->op, "/") == 0 || strcmp(node->op, "%") == 0)
        {
            if (is_numeric(left_type) && is_numeric(right_type))
            {
                return (strcmp(left_type, "float") == 0 || strcmp(right_type, "float") == 0) ? "float" : "int";
            }
            semantic_error(node->line, "invalid expression: arithmetic operators require numeric operands");
            (*error_count)++;
            return "unknown";
        }

        if (strcmp(node->op, "<") == 0 || strcmp(node->op, ">") == 0 || strcmp(node->op, "<=") == 0 || strcmp(node->op, ">=") == 0 || strcmp(node->op, "==") == 0 || strcmp(node->op, "!=") == 0)
        {
            if (is_numeric(left_type) && is_numeric(right_type))
            {
                return "bool";
            }
            if (strcmp(left_type, "bool") == 0 && strcmp(right_type, "bool") == 0 && (strcmp(node->op, "==") == 0 || strcmp(node->op, "!=") == 0))
            {
                return "bool";
            }
            semantic_error(node->line, "invalid expression: relational operators require compatible operands");
            (*error_count)++;
            return "unknown";
        }

        return "unknown";
    }

    default:
        return "unknown";
    }
}

static void analyze_stmt_list(ASTNode *list, SymbolTable *table, int *error_count)
{
    for (ASTNode *cur = list; cur != NULL; cur = cur->next)
    {
        if (!cur)
            continue;
        switch (cur->type)
        {
        case NODE_DECL:
        {
            if (!symtab_insert(table, cur->decl_name, cur->decl_type, cur->line))
            {
                char buffer[256];
                snprintf(buffer, sizeof(buffer), "redeclaration of '%s' in the same scope", cur->decl_name);
                semantic_error(cur->line, buffer);
                (*error_count)++;
            }
            break;
        }

        case NODE_ASSIGN:
        {
            Symbol *sym = symtab_lookup_active(table, cur->assign_name);
            if (!sym)
            {
                Symbol *inactive = symtab_lookup_any(table, cur->assign_name);
                if (inactive)
                {
                    semantic_error(cur->line, "scope violation: variable is not visible outside its block");
                    (*error_count)++;
                }
                else
                {
                    char buffer[256];
                    snprintf(buffer, sizeof(buffer), "undeclared variable '%s'", cur->assign_name);
                    semantic_error(cur->line, buffer);
                    (*error_count)++;
                }
            }
            else
            {
                const char *expr_type = infer_expr_type(cur->assign_expr, table, error_count);
                if (strcmp(expr_type, "unknown") != 0)
                {
                    if (is_assignable(sym->type, expr_type))
                    {
                        break;
                    }

                    if (strcmp(sym->type, "bool") == 0 || strcmp(expr_type, "bool") == 0)
                    {
                        semantic_error(cur->line, "invalid assignment: boolean values require boolean expressions");
                        (*error_count)++;
                    }
                    else
                    {
                        char buffer[256];
                        snprintf(buffer, sizeof(buffer), "type mismatch: cannot assign '%s' to '%s'", expr_type, sym->type);
                        semantic_error(cur->line, buffer);
                        (*error_count)++;
                    }
                }
            }
            break;
        }

        case NODE_IF:
        {
            const char *cond_type = infer_expr_type(cur->if_cond, table, error_count);
            if (strcmp(cond_type, "bool") != 0 && strcmp(cond_type, "unknown") != 0)
            {
                semantic_error(cur->line, "invalid expression: if-condition must be boolean");
                (*error_count)++;
            }
            if (cur->if_then)
            {
                symtab_enter_scope(table);
                analyze_stmt_list(cur->if_then->block_stmts, table, error_count);
                symtab_exit_scope(table);
            }
            if (cur->if_else)
            {
                symtab_enter_scope(table);
                analyze_stmt_list(cur->if_else->block_stmts, table, error_count);
                symtab_exit_scope(table);
            }
            break;
        }

        case NODE_WHILE:
        {
            const char *cond_type = infer_expr_type(cur->while_cond, table, error_count);
            if (strcmp(cond_type, "bool") != 0 && strcmp(cond_type, "unknown") != 0)
            {
                semantic_error(cur->line, "invalid expression: while-condition must be boolean");
                (*error_count)++;
            }
            if (cur->while_body)
            {
                symtab_enter_scope(table);
                analyze_stmt_list(cur->while_body->block_stmts, table, error_count);
                symtab_exit_scope(table);
            }
            break;
        }

        case NODE_PRINT:
        {
            (void)infer_expr_type(cur->print_expr, table, error_count);
            break;
        }

        case NODE_BLOCK:
        {
            symtab_enter_scope(table);
            analyze_stmt_list(cur->block_stmts, table, error_count);
            symtab_exit_scope(table);
            break;
        }

        default:
            break;
        }
    }
}

int semantic_analyze(ASTNode *root)
{
    SymbolTable *table = symtab_create();
    int error_count = 0;

    analyze_stmt_list(root, table, &error_count);

    symtab_destroy(table);
    return error_count;
}

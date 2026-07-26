#ifndef AST_H
#define AST_H


typedef enum {
    NODE_DECL,
    NODE_ASSIGN,
    NODE_IF,
    NODE_WHILE,
    NODE_PRINT,
    NODE_BLOCK,
    NODE_BINOP,
    NODE_UNOP,
    NODE_IDENTIFIER,
    NODE_INT_LIT,
    NODE_FLOAT_LIT,
    NODE_BOOL_LIT
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    int line;


    char *decl_type;              
    char *decl_name;


    char *assign_name;
    struct ASTNode *assign_expr;

    struct ASTNode *if_cond;
    struct ASTNode *if_then;
    struct ASTNode *if_else;     
    /* NODE_WHILE */
    struct ASTNode *while_cond;
    struct ASTNode *while_body;


    struct ASTNode *print_expr;


    struct ASTNode *block_stmts;


    char *op;                     /* "+", "-", "&&", "!", ... */
    struct ASTNode *left;         /* also used as the sole operand for unop */
    struct ASTNode *right;        /* unused for unop */


    char *name;

    /* Literal values */
    int   int_val;
    float float_val;
    int   bool_val;


    struct ASTNode *next;
} ASTNode;


ASTNode *ast_make_decl(const char *type, const char *name, int line);
ASTNode *ast_make_assign(const char *name, ASTNode *expr, int line);
ASTNode *ast_make_if(ASTNode *cond, ASTNode *then_b, ASTNode *else_b, int line);
ASTNode *ast_make_while(ASTNode *cond, ASTNode *body, int line);
ASTNode *ast_make_print(ASTNode *expr, int line);
ASTNode *ast_make_block(ASTNode *stmts, int line);
ASTNode *ast_make_binop(const char *op, ASTNode *l, ASTNode *r, int line);
ASTNode *ast_make_unop(const char *op, ASTNode *operand, int line);
ASTNode *ast_make_identifier(const char *name, int line);
ASTNode *ast_make_int_lit(int val, int line);
ASTNode *ast_make_float_lit(float val, int line);
ASTNode *ast_make_bool_lit(int val, int line);

ASTNode *ast_append_stmt(ASTNode *list, ASTNode *stmt);


void ast_print(ASTNode *node, int indent);
void ast_print_list(ASTNode *list, int indent);

#endif

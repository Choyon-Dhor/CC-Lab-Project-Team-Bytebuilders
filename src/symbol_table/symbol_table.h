#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

typedef struct Symbol {
    char *name;
    char *type;
    int line;
    int active;
    int scope_depth;
    struct Symbol *next;
} Symbol;

typedef struct Scope {
    int depth;
    struct Scope *parent;
    Symbol *symbols;
} Scope;

typedef struct SymbolTable {
    Scope *current_scope;
    Symbol *inactive_symbols;
    int scope_depth;
} SymbolTable;

SymbolTable *symtab_create(void);
void symtab_destroy(SymbolTable *table);

void symtab_enter_scope(SymbolTable *table);
void symtab_exit_scope(SymbolTable *table);

int symtab_insert(SymbolTable *table, const char *name, const char *type, int line);
Symbol *symtab_lookup_active(SymbolTable *table, const char *name);
Symbol *symtab_lookup_any(SymbolTable *table, const char *name);
int symtab_is_declared_in_current_scope(SymbolTable *table, const char *name);

void symtab_print(SymbolTable *table);

#endif

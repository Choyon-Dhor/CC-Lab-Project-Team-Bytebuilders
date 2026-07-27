#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symbol_table.h"

static char *xstrdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (!copy) {
        fprintf(stderr, "Fatal: out of memory while duplicating symbol name\n");
        exit(1);
    }
    memcpy(copy, s, len);
    return copy;
}

SymbolTable *symtab_create(void) {
    SymbolTable *table = calloc(1, sizeof(SymbolTable));
    if (!table) {
        fprintf(stderr, "Fatal: out of memory while creating symbol table\n");
        exit(1);
    }
    symtab_enter_scope(table);
    return table;
}

void symtab_destroy(SymbolTable *table) {
    if (!table) return;

    while (table->current_scope) {
        Scope *scope = table->current_scope;
        table->current_scope = scope->parent;

        while (scope->symbols) {
            Symbol *sym = scope->symbols;
            scope->symbols = sym->next;
            free(sym->name);
            free(sym->type);
            free(sym);
        }

        free(scope);
    }

    while (table->inactive_symbols) {
        Symbol *sym = table->inactive_symbols;
        table->inactive_symbols = sym->next;
        free(sym->name);
        free(sym->type);
        free(sym);
    }

    free(table);
}

void symtab_enter_scope(SymbolTable *table) {
    if (!table) return;

    Scope *scope = calloc(1, sizeof(Scope));
    if (!scope) {
        fprintf(stderr, "Fatal: out of memory while creating symbol scope\n");
        exit(1);
    }

    scope->depth = table->scope_depth;
    scope->parent = table->current_scope;
    table->current_scope = scope;
    table->scope_depth++;
}

void symtab_exit_scope(SymbolTable *table) {
    if (!table || !table->current_scope) return;

    Scope *scope = table->current_scope;
    table->current_scope = scope->parent;
    table->scope_depth--;

    while (scope->symbols) {
        Symbol *sym = scope->symbols;
        scope->symbols = sym->next;
        sym->active = 0;
        sym->next = table->inactive_symbols;
        table->inactive_symbols = sym;
    }

    free(scope);
}

int symtab_insert(SymbolTable *table, const char *name, const char *type, int line) {
    if (!table || !table->current_scope || !name || !type) return 0;

    if (symtab_is_declared_in_current_scope(table, name)) {
        return 0;
    }

    Symbol *sym = calloc(1, sizeof(Symbol));
    if (!sym) {
        fprintf(stderr, "Fatal: out of memory while inserting symbol\n");
        exit(1);
    }

    sym->name = xstrdup(name);
    sym->type = xstrdup(type);
    sym->line = line;
    sym->active = 1;
    sym->scope_depth = table->scope_depth;
    sym->next = table->current_scope->symbols;
    table->current_scope->symbols = sym;
    return 1;
}

Symbol *symtab_lookup_active(SymbolTable *table, const char *name) {
    if (!table || !name) return NULL;

    for (Scope *scope = table->current_scope; scope != NULL; scope = scope->parent) {
        for (Symbol *sym = scope->symbols; sym != NULL; sym = sym->next) {
            if (strcmp(sym->name, name) == 0 && sym->active) {
                return sym;
            }
        }
    }

    return NULL;
}

Symbol *symtab_lookup_any(SymbolTable *table, const char *name) {
    if (!table || !name) return NULL;

    for (Scope *scope = table->current_scope; scope != NULL; scope = scope->parent) {
        for (Symbol *sym = scope->symbols; sym != NULL; sym = sym->next) {
            if (strcmp(sym->name, name) == 0 && sym->active) {
                return sym;
            }
        }
    }

    for (Symbol *sym = table->inactive_symbols; sym != NULL; sym = sym->next) {
        if (strcmp(sym->name, name) == 0) {
            return sym;
        }
    }

    return NULL;
}

int symtab_is_declared_in_current_scope(SymbolTable *table, const char *name) {
    if (!table || !table->current_scope || !name) return 0;

    for (Symbol *sym = table->current_scope->symbols; sym != NULL; sym = sym->next) {
        if (strcmp(sym->name, name) == 0) {
            return 1;
        }
    }

    return 0;
}

void symtab_print(SymbolTable *table) {
    if (!table) return;

    printf("\n[Symbol Table]\n");
    for (Scope *scope = table->current_scope; scope != NULL; scope = scope->parent) {
        printf("Scope depth %d\n", scope->depth);
        for (Symbol *sym = scope->symbols; sym != NULL; sym = sym->next) {
            printf("  - %s : %s (line %d)\n", sym->name, sym->type, sym->line);
        }
    }
}

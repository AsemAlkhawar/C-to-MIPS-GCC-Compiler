#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"

extern int chk_decl_flag; // Declared in driver.c

SymbolTable symbolTableStack[MAX_SCOPES];
int currentScope = -1;

void print_symbol_table_stack() {
    printf("Symbol Table Stack:\n");
    for (int scopeIndex = 0; scopeIndex <= currentScope; scopeIndex++) {
        SymbolTable* symbolTable = &symbolTableStack[scopeIndex];
        printf("Scope %d: (size: %d)\n", scopeIndex, symbolTable->size);

        for (int entryIndex = 0; entryIndex < symbolTable->size; entryIndex++) {
            SymbolTableEntry entry = symbolTable->entries[entryIndex];
            char* typeName = (entry.type == FUNCTION) ? "Function" : "Variable";
            printf("  Entry %d: Name: %s, Type: %s", entryIndex, entry.name, typeName);

            if (entry.type == FUNCTION) {
                printf(", Argument Count: %d", entry.argCount);
            }

            printf("\n");
        }
    }
}


void push_scope() {
    currentScope++;
    symbolTableStack[currentScope].entries = malloc(MAX_IDENTIFIERS * sizeof(SymbolTableEntry));
    symbolTableStack[currentScope].size = 0;
    symbolTableStack[currentScope].capacity = MAX_IDENTIFIERS; // Initialize capacity
    if (!symbolTableStack[currentScope].entries) {
        // Handle malloc failure
        exit(1); 
    }
}

void pop_scope() {
    if (currentScope >= 0) {
        free(symbolTableStack[currentScope].entries); // Free the current scope's entries
        currentScope--;
    }
}

int add_to_symbol_table(char* name, IdType type, Scope scope) {

    // Check if the symbol table is uninitialized and initialize the first scope if necessary
    if (currentScope == -1) {
        push_scope(); // This will set currentScope to 0 and allocate memory for the first scope's entries
    }

    SymbolTable* currentSymbolTable = &symbolTableStack[currentScope];

    for (int i = 0; i < currentSymbolTable->size; i++) {
        if (strcmp(currentSymbolTable->entries[i].name, name) == 0) {
            return -1; // Entry already exists
        }
    }

    if (currentSymbolTable->size >= currentSymbolTable->capacity) {
        // Handle reallocation if needed
        fprintf(stderr, "Capacity insuffiecient");
    }

    char* duplicatedName = strdup(name);
    if (!duplicatedName) {
        // Handle strdup failure
        return -2; // Memory allocation error
    }

    currentSymbolTable->entries[currentSymbolTable->size].name = duplicatedName;
    currentSymbolTable->entries[currentSymbolTable->size].type = type;
    currentSymbolTable->size++;

    return 0; // Success
}


int check_function_call(char* name, int argCount) {

    if (chk_decl_flag == 0) {
        return 0;
    }

    //print_symbol_table_stack();

    for (int scope = currentScope; scope >= -1; scope--) {
        SymbolTable* symbolTable = &symbolTableStack[scope];
        for (int i = 0; i < symbolTable->size; i++) {
            if (strcmp(symbolTable->entries[i].name, name) == 0 && symbolTable->entries[i].type == FUNCTION) {
                // Check if the number of arguments matches
                if (symbolTable->entries[i].argCount == argCount) {
                    if (check_duplicate_variable_in_scope(name, 1) == 1) {
                        return -3; // wrong type
                    }
                    return 0;
                }
                else {
                    return -2; // Incorrect number of arguments
                }
            }
        }
    }

    return -1; // Function not declared
}

int check_duplicate_variable_in_scope(char* name, int scope) {
    SymbolTable* symbolTable = &symbolTableStack[scope];

    for (int i = 0; i < symbolTable->size; i++) {
        if (strcmp(symbolTable->entries[i].name, name) == 0 && symbolTable->entries[i].type == VARIABLE) {
            return 1;
        }
    }

    return 0;
}


int check_variable_declaration(char* name) {




    for (int scope = currentScope; scope >= 0; scope--) {
        SymbolTable* symbolTable = &symbolTableStack[scope];
        for (int i = 0; i < symbolTable->size; i++) {
            if (strcmp(symbolTable->entries[i].name, name) == 0 && symbolTable->entries[i].type == VARIABLE) {
                return 0; // Variable declared
            }
        }
    }
    if (chk_decl_flag == 0) {
        return 0;
    }
    return -1; // Variable not declared
}


int update_function_arg_count(char* name, int argCount) {


    for (int scope = currentScope; scope >= 0; scope--) {
        SymbolTable* symbolTable = &symbolTableStack[scope];
        for (int i = 0; i < symbolTable->size; i++) {
            if (strcmp(symbolTable->entries[i].name, name) == 0) {
                    symbolTable->entries[i].argCount = argCount;
                
            }
        }
    }
    return -1; // Function not declared
}


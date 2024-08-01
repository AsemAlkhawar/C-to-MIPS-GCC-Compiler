#pragma once
#include "ast.h"
#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    CALL, 
    OP_FUNC_DEF, 
    OP_PARAM,
    OP_LABEL,
    OP_ASSIGN,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_GOTO,
    OP_GT, OP_LT, OP_GE, OP_LE, OP_EQ, OP_NE,
    OP_IF_GOTO,
    OP_AND, OP_OR,
    OP_UMINUS,




    OP_RETURN,
    OP_RETURN_CALLER


} OpCode;




typedef struct ThreeAddrInstr {
    OpCode op;            // Operation code
    char* arg1;           // First operand
    char* arg2;           // Second operand, if applicable
    char* result;      // Result variable or label for control flow
    struct ThreeAddrInstr* next; // Next instruction in the list
    int ArgCount;
    char** argnames;// Array of argument names
} ThreeAddrInstr;

static int tmpCounter = 1; // Global counter for temporary variable names

ThreeAddrInstr* newInstruction(OpCode op, const char* arg1, const char* arg2, const char* result);
IdentifierNode* createIdentifierNode(const char* name);
ThreeAddrInstr* generateInstructions(void* astNode);
ThreeAddrInstr* generateInstructionsRec(void* astNode, ThreeAddrInstr** lastInstruction);
char* generateTmpVar();
void traverseAndComposeInstructions(void* astNode, ThreeAddrInstr** lastInstruction);
void AppendInstruction(ThreeAddrInstr* instr);
char* arith_instr(void* astNode, ThreeAddrInstr** lastInstruction);
void printFileContents();
void print3ACInstruction(ThreeAddrInstr* instr);

int populateVariables(char** argnames, int count);
char* getVariableAddress(const char* varName);
void generateMIPSCode(ThreeAddrInstr* instr_list);
void initializeArrays();
void PrintMIPSFromThreeAddrInstrs(ThreeAddrInstr* instr_list);
void generateDataSection(FILE* outputFile);
void appendFileContentsToFile(FILE* outputFile);
void printFileContents();
void print(FILE* file, const char* format, ...);
char* bool_Instr(void* astNode);
char* generateArithHolder();
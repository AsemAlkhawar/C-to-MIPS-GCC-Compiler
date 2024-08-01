#include "gen_code.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


#define MAX_PARAMS 10
#define MAX_LOCALS 10
#define MAX_GLOBALS 10
#define MAX_SAVED_REGISTERS 8

static int paramIndex = 0; // Global variable to keep track of the parameter index
int maxSize;
// Arrays for parameters and their addresses
char* params[MAX_PARAMS];
int params_address[MAX_PARAMS];

// Arrays for local variables and their addresses
char* locals[MAX_LOCALS];
int locals_address[MAX_LOCALS];

// Array for global variables
char* globals[MAX_GLOBALS];

// Array for saved registers
char* saved_registers[MAX_SAVED_REGISTERS];


void print(FILE* file, const char* format, ...) {
    va_list args;
    va_start(args, file);

    // Print to stdout
    vprintf(format, args);

    // Print to file
    vfprintf(file, format, args);

    va_end(args);
}


int populateVariables(char** argnames, int count) {

    SymbolTable* globalSymbolTable = &symbolTableStack[0];
    SymbolTable* localSymbolTable = &symbolTableStack[1];

    // Populate params array
    int paramIndex = 0;
    while (count != 0 && argnames[paramIndex] != NULL) {
        params[paramIndex] = strdup(argnames[paramIndex]);
        paramIndex++;
        count--;
    }

    // Populate globals array
    int globalIndex = 0;
    for (int i = 0; i < globalSymbolTable->size; i++) {
        if (globalSymbolTable->entries[i].type == VARIABLE) {
            // Check if the global variable is also a local variable
            int isLocal = 0;
            for (int j = 0; j < localSymbolTable->size; j++) {
                if (strcmp(globalSymbolTable->entries[i].name, localSymbolTable->entries[j].name) == 0) {
                    isLocal = 1;
                    break;
                }
            }
            if (!isLocal) {
                globals[globalIndex] = strdup(globalSymbolTable->entries[i].name);
                globalIndex++;
            }
        }
    }

    // Populate locals array
    int localIndex = 0;
    for (int i = 0; i < localSymbolTable->size; i++) {
        if (localSymbolTable->entries[i].type == VARIABLE) {
            locals[localIndex] = strdup(localSymbolTable->entries[i].name);
            localIndex++;
        }
    }


    int maxSize = 32 + 4 * localIndex;

    // For params address array
    for (int i = 0; i < paramIndex; i++) {
        params_address[i] = maxSize + 4 * i;
    }

    // For locals address array
    for (int i = 0; i < localIndex; i++) {
        locals_address[i] = maxSize - 16 - 4 * i;
    }

    return maxSize + 32;
}

char* getVariableAddress(const char* varName) {
    // Check locals
    for (int i = 0; i < MAX_LOCALS; i++) {
        if (locals[i] != NULL && strcmp(locals[i], varName) == 0) {
            char* addressStr = (char*)malloc(20 * sizeof(char));
            sprintf(addressStr, "%d($fp)", locals_address[i]);
            return addressStr;
        }
    }

    // Check params
    for (int i = 0; i < MAX_PARAMS; i++) {
        if (params[i] != NULL && strcmp(params[i], varName) == 0) {
            char* addressStr = (char*)malloc(20 * sizeof(char));
            sprintf(addressStr, "%d($fp)", params_address[i]);
            return addressStr;
        }
    }

    // Check globals
    for (int i = 0; i < MAX_GLOBALS; i++) {
        if (globals[i] != NULL && strcmp(globals[i], varName) == 0) {
            int length = strlen(varName) + 6 + 1;
            char* addressStr = (char*)malloc(length * sizeof(char));

            if (addressStr != NULL) {
                sprintf(addressStr, "_1_%s_1_", varName);
            }

            return addressStr;
        }
    }

    if (strncmp(varName, "__temp__", 8) == 0) {

        // Check if the variable is already assigned to one of the saved registers
        for (int i = 0; i < MAX_SAVED_REGISTERS; i++) {
            if (saved_registers[i] != NULL && strcmp(saved_registers[i], varName) == 0) {
                char* addressStr = (char*)malloc(20 * sizeof(char));

                sprintf(addressStr, "$s%d", i);
                saved_registers[i] = NULL; // Erase the register assignment
                return addressStr;

            }
        }

        // If the variable is not found, assign it to the first available NULL register
        for (int i = 0; i < MAX_SAVED_REGISTERS; i++) {
            if (saved_registers[i] == NULL) {
                saved_registers[i] = strdup(varName); // Allocate and assign the variable name
                char* addressStr = (char*)malloc(20 * sizeof(char));
                sprintf(addressStr, "$s%d", i);
                return addressStr;
            }
        }
    }

    // If not found, return NULL
    return NULL;
}

// Function to generate MIPS code from the linked list of instructions
void generateMIPSCode(ThreeAddrInstr* instr_list) {
    FILE* file = fopen("./MIPS_CODE.s", "a");

    ThreeAddrInstr* current_instr = instr_list;
    char* resultAddress;

    char* paramAddress;

    while (current_instr != NULL) {
        switch (current_instr->op) {
        case OP_FUNC_DEF:
            maxSize = populateVariables(current_instr->argnames, current_instr->ArgCount);

            printf("%s:\n", current_instr->result); // Function name
            printf("\taddiu\t$sp, $sp, -%d\n", maxSize); // Adjust stack pointer
            printf("\tsw\t$31, %d($sp)\n", maxSize - 4); // Save return address
            printf("\tsw\t$fp, %d($sp)\n", maxSize - 8); // Save frame pointer

            for (int i = 0; i < 8; i++) {
                printf("\tsw\t$s%d, %d($sp)\t# Store $s%d at offset %d from the stack pointer\n", i, maxSize - 12 - 4 * i, i, maxSize - 12 - 4 * i);
            }

            printf("\tmove\t$fp, $sp\n"); // Update frame pointer

            fprintf(file, "%s:\n", current_instr->result); // Function name
            fprintf(file, "\taddiu\t$sp, $sp, -%d\n", maxSize); // Adjust stack pointer
            fprintf(file, "\tsw\t$31, %d($sp)\n", maxSize - 4); // Save return address
            fprintf(file, "\tsw\t$fp, %d($sp)\n", maxSize - 8); // Save frame pointer

            for (int i = 0; i < 8; i++) {
                fprintf(file, "\tsw\t$s%d, %d($sp)\t# Store $s%d at offset %d from the stack pointer\n", i, maxSize - 12 - 4 * i, i, maxSize - 12 - 4 * i);
            }

            fprintf(file, "\tmove\t$fp, $sp\n"); // Update frame pointer
            // Handle parameters
            char** tmp = current_instr->argnames;
            for (int i = 0; i < current_instr->ArgCount; i++) {
                printf("\tsw\t$%d, %s\n", 4 + i, getVariableAddress(*tmp)); // Save parameters
                fprintf(file, "\tsw\t$%d, %s\n", 4 + i, getVariableAddress(*tmp));
                tmp++;
            }
            break;
        case OP_ASSIGN: {
            // Determine if the result is a local, global, or a register
            char* result = getVariableAddress(current_instr->result);
            char* arg1_ = getVariableAddress(current_instr->arg1);

            if (strncmp(current_instr->result, "__temp__", 8) == 0) {
                // Result is a register
                if (isdigit(current_instr->arg1[0])) {
                    // RHS is an integer constant
                    fprintf(file, "\tli\t%s, %s\n", result, current_instr->arg1);
                    printf("\tli\t%s, %s\n", result, current_instr->arg1);
                }
                else {
                    // RHS is a local or global variable
                    fprintf(file, "\tlw\t%s, %s\n", result, arg1_);
                    printf("\tlw\t%s, %s\n", result, arg1_); 
                }
            }
            else {
                // Result is a local or global variable
                if (strncmp(current_instr->arg1, "__temp__", 8) == 0) {
                    // RHS is a register
                    fprintf(file, "\tsw\t%s, %s\n", arg1_, result);
                    printf("\tsw\t%s, %s\n", arg1_, result); // Store RHS register to result variable
                }
                else {
                    if (isdigit(current_instr->arg1[0])) {
                        // RHS is an integer constant
                        printf("ERROR\tli\t%s, %s\n", getVariableAddress(current_instr->result), current_instr->arg1); 
                        fprintf(file, "ERROR\tli\t%s, %s\n", getVariableAddress(current_instr->result), current_instr->arg1);
                    }
                    else {
                        // RHS is also a local or global variable
                        printf("ERROR\tlw\t$2, %s\n", arg1_); 
                        fprintf(file, "ERROR\tlw\t$2, %s\n", arg1_); 
                    }
                    printf("ERROR\tsw\t$2, %s\n", result); 
                    fprintf(file, "ERROR\tsw\t$2, %s\n", result);
                }
            }

            // Free allocated memory for result address
            free(result);
            break;

        }
        case OP_PARAM: {
            char* result = getVariableAddress(current_instr->arg1);
            printf("\tmove\t$%d,%s\n", paramIndex + 4, result);
            fprintf(file, "\tmove\t$%d,%s\n", paramIndex + 4, result);
            paramIndex++; // Increment parameter index for the next parameter
            break;
        }


        case CALL:
            fprintf(file, "\tjal\t%s\n", current_instr->arg1);
            printf("\tjal\t%s\n", current_instr->arg1); // Function call
            paramIndex = 0;
            break;

        //case OP_BOOL: {
        //    // Load the first operand into $3
        //    if (isdigit(current_instr->arg1[0]) || (current_instr->arg1[0] == '-' && isdigit(current_instr->arg1[1]))) {
        //        // Operand is an integer constant
        //        printf("\tli\t$3, %s\n", current_instr->arg1);
        //        fprintf(file, "\tli\t$3, %s\n", current_instr->arg1);
        //    }
        //    else {
        //        // Operand is an identifier
        //        char* addr1 = getVariableAddress(current_instr->arg1);
        //        printf("\tlw\t$3, %s\n", addr1);
        //        fprintf(file, "\tlw\t$3, %s\n", addr1);
        //        free(addr1);
        //    }

        //    // Load the second operand into $2
        //    if (isdigit(current_instr->arg2[0]) || (current_instr->arg2[0] == '-' && isdigit(current_instr->arg2[1]))) {
        //        // Operand is an integer constant
        //        printf("\tli\t$2, %s\n", current_instr->arg2);
        //        fprintf(file, "\tli\t$2, %s\n", current_instr->arg2);
        //    }
        //    else {
        //        // Operand is an identifier
        //        char* addr2 = getVariableAddress(current_instr->arg2);
        //        printf("\tlw\t$2, %s\n", addr2);
        //        fprintf(file, "\tlw\t$2, %s\n", addr2);
        //        free(addr2);
        //    }


        //    if (current_instr->ArgCount == 1) {
        //        if (strcmp(current_instr->result, ">") == 0) {
        //            printf("\tslt\t$2, $2, $3\n\tbne\t$2, $0, ");
        //            fprintf(file, "\tslt\t$2, $2, $3\n\tbne\t$2, $0, ");
        //        }
        //        else if (strcmp(current_instr->result, "<") == 0) {
        //            printf("\tslt\t$2, $3, $2\n\tbne\t$2, $0, ");
        //            fprintf(file, "\tslt\t$2, $3, $2\n\tbne\t$2, $0, ");
        //        }
        //        else if (strcmp(current_instr->result, ">=") == 0) {
        //            printf("\tslt\t$2, $3, $2\n\tbeq\t$2, $0, ");
        //            fprintf(file, "\tslt\t$2, $3, $2\n\tbeq\t$2, $0, ");
        //        }
        //        else if (strcmp(current_instr->result, "<=") == 0) {
        //            printf("\tslt\t$2, $2, $3\n\tbeq\t$2, $0, ");
        //            fprintf(file, "\tslt\t$2, $2, $3\n\tbeq\t$2, $0, ");
        //        }
        //        else if (strcmp(current_instr->result, "==") == 0) {
        //            printf("\tbeq\t$3, $2, ");
        //            fprintf(file, "\tbeq\t$3, $2, ");
        //        }
        //        else if (strcmp(current_instr->result, "!=") == 0) {
        //            printf("\tbne\t$3, $2, ");
        //            fprintf(file, "\tbne\t$3, $2, ");
        //        }
        //    }
        //    else {
        //        if (strcmp(current_instr->result, ">") == 0) {
        //            printf("\tslt\t$2, $2, $3\n\tbeq\t$2, $0, ");
        //            fprintf(file, "\tslt\t$2, $2, $3\n\tbeq\t$2, $0, ");
        //        }
        //        else if (strcmp(current_instr->result, "<") == 0) {
        //            printf("\tslt\t$2, $3, $2\n\tbeq\t$2, $0, ");
        //            fprintf(file, "\tslt\t$2, $3, $2\n\tbeq\t$2, $0, ");
        //        }
        //        else if (strcmp(current_instr->result, ">=") == 0) {
        //            printf("\tslt\t$2, $3, $2\n\tbne\t$2, $0, ");
        //            fprintf(file, "\tslt\t$2, $3, $2\n\tbne\t$2, $0, ");
        //        }
        //        else if (strcmp(current_instr->result, "<=") == 0) {
        //            printf("\tslt\t$2, $2, $3\n\tbne\t$2, $0, ");
        //            fprintf(file, "\tslt\t$2, $2, $3\n\tbne\t$2, $0, ");
        //        }
        //        else if (strcmp(current_instr->result, "==") == 0) {
        //            printf("\tbne\t$3, $2, ");
        //            fprintf(file, "\tbne\t$3, $2, ");
        //        }
        //        else if (strcmp(current_instr->result, "!=") == 0) {
        //            printf("\tbeq\t$3, $2, ");
        //            fprintf(file, "\tbeq\t$3, $2, ");
        //        }
        //    }

        //    break;
        //}
        case OP_RETURN: {
            char* arg1 = getVariableAddress(current_instr->arg1);

            printf("\tmove\t$t9, %s\n", arg1);
            fprintf(file, "\tmove\t$t9, %s\n", arg1);
            break;
        }
        case OP_RETURN_CALLER: {
            char* result = getVariableAddress(current_instr->result);

            printf("\tmove\t%s, $t9\n", result);
            fprintf(file, "\tmove\t%s, $t9\n", result);
            break;
        }
        case OP_UMINUS: {
            char* result = getVariableAddress(current_instr->result);
            char* arg1 = getVariableAddress(current_instr->arg1);
            
            printf("\tneg\t%s,%s\n", result, arg1);
            fprintf(file, "\tneg\t%s,%s\n", result, arg1);
            break;
        }
        case OP_AND: {
            char* result = getVariableAddress(current_instr->result);
            char* arg1 = getVariableAddress(current_instr->arg1);
            char* arg2 = getVariableAddress(current_instr->arg2);

            printf("\tand\t%s, %s, %s\n", result, arg1, arg2);
            fprintf(file, "\tand\t%s, %s, %s\n", result, arg1, arg2);
            break;
        }
        case OP_OR: {
            char* result = getVariableAddress(current_instr->result);
            char* arg1 = getVariableAddress(current_instr->arg1);
            char* arg2 = getVariableAddress(current_instr->arg2);

            printf("\tor\t%s, %s, %s\n", result, arg1, arg2);
            fprintf(file, "\tor\t%s, %s, %s\n", result, arg1, arg2);
            break;
        }
        case OP_IF_GOTO: {
            char* exprResult = getVariableAddress(current_instr->arg1);

            if (current_instr->result) {


                printf("\tli\t$t8, 1\n");
                printf("\tbeq\t%s, $t8, %s\n", exprResult, current_instr->result);
                fprintf(file, "\tli\t$t8, 1\n");
                fprintf(file,"\tbeq\t%s, $t8, %s\n", exprResult, current_instr->result);
            }
            else {


                printf("\tbeqz\t%s, %s\n", exprResult, current_instr->arg2);
                fprintf(file, "\tbeqz\t%s, %s\n", exprResult, current_instr->arg2);
            }
            break;
        }

        case OP_LABEL:
            printf("%s:\n", current_instr->result);
            fprintf(file, "%s:\n", current_instr->result);
            break;
        case OP_GOTO:
            printf("\tb\t%s\n", current_instr->result);
            fprintf(file, "\tb\t%s\n", current_instr->result);
            break;
        case OP_ADD: {
            char* result = getVariableAddress(current_instr->result);
            char* arg1 = getVariableAddress(current_instr->arg1);
            char* arg2 = getVariableAddress(current_instr->arg2);
            printf("\taddu\t%s,%s,%s\n", result, arg1, arg2);
            fprintf(file, "\taddu\t%s,%s,%s\n", result, arg1, arg2);
            break;
        }

        case OP_SUB: {
            char* result = getVariableAddress(current_instr->result);
            char* arg1 = getVariableAddress(current_instr->arg1);
            char* arg2 = getVariableAddress(current_instr->arg2);
            printf("\tsubu\t%s,%s,%s\n", result, arg1, arg2);
            fprintf(file, "\tsubu\t%s,%s,%s\n", result, arg1, arg2);
            break;
        }

        case OP_MUL: {
            char* result = getVariableAddress(current_instr->result);
            char* arg1 = getVariableAddress(current_instr->arg1);
            char* arg2 = getVariableAddress(current_instr->arg2);
            printf("\tmul\t%s, %s, %s\n", result, arg1, arg2);
            fprintf(file, "\tmul\t%s, %s, %s\n", result, arg1, arg2);
            break;
        }

        case OP_DIV: {
            char* result = getVariableAddress(current_instr->result);
            char* arg1 = getVariableAddress(current_instr->arg1);
            char* arg2 = getVariableAddress(current_instr->arg2);
            printf("\tdiv\t%s, %s, %s\n", result, arg1, arg2);
            fprintf(file, "\tdiv\t%s, %s, %s\n", result, arg1, arg2);
            break;
        }
        case OP_GT: {
            char* result = getVariableAddress(current_instr->result);
            char* arg1 = getVariableAddress(current_instr->arg1);
            char* arg2 = getVariableAddress(current_instr->arg2);
            printf("\tsgt\t%s, %s, %s\n", result, arg1, arg2);
            fprintf(file, "\tsgt\t%s, %s, %s\n", result, arg1, arg2);
            break;
        }
        case OP_GE: {
            char* result = getVariableAddress(current_instr->result);
            char* arg1 = getVariableAddress(current_instr->arg1);
            char* arg2 = getVariableAddress(current_instr->arg2);
            printf("\tsge\t%s, %s, %s\n", result, arg1, arg2);
            fprintf(file, "\tsge\t%s, %s, %s\n", result, arg1, arg2);
            break;
        }
        case OP_LT: {
            char* result = getVariableAddress(current_instr->result);
            char* arg1 = getVariableAddress(current_instr->arg1);
            char* arg2 = getVariableAddress(current_instr->arg2);
            printf("\tslt\t%s, %s, %s\n", result, arg1, arg2);
            fprintf(file, "\tslt\t%s, %s, %s\n", result, arg1, arg2);
            break;
        }
        case OP_LE: {
            char* result = getVariableAddress(current_instr->result);
            char* arg1 = getVariableAddress(current_instr->arg1);
            char* arg2 = getVariableAddress(current_instr->arg2);
            printf("\tsle\t%s, %s, %s\n", result, arg1, arg2);
            fprintf(file, "\tsle\t%s, %s, %s\n", result, arg1, arg2);
            break;
        }
        case OP_EQ: {
            char* result = getVariableAddress(current_instr->result);
            char* arg1 = getVariableAddress(current_instr->arg1);
            char* arg2 = getVariableAddress(current_instr->arg2);
            printf("\tseq\t%s, %s, %s\n", result, arg1, arg2);
            fprintf(file, "\tseq\t%s, %s, %s\n", result, arg1, arg2);
            break;
        }
        case OP_NE: {
            char* result = getVariableAddress(current_instr->result);
            char* arg1 = getVariableAddress(current_instr->arg1);
            char* arg2 = getVariableAddress(current_instr->arg2);
            printf("\tsne\t%s, %s, %s\n", result, arg1, arg2);
            fprintf(file, "\tsne\t%s, %s, %s\n", result, arg1, arg2);
            break;
        }

        default:
            break;
        }
        current_instr = current_instr->next; // Move to the next instruction
    }

    printf("\tmove\t$sp, $fp\n"); // Update frame pointer
    printf("\tlw\t$31, %d($sp)\n", maxSize - 4); // Save return address
    printf("\tlw\t$fp, %d($sp)\n", maxSize - 8); // Save frame pointer

    for (int i = 0; i < 8; i++) {
        printf("\tlw\t$s%d, %d($sp)\t# Load $s%d at offset %d from the stack pointer\n", i, maxSize - 12 - 4 * i, i, maxSize - 12 - 4 * i);
    }

    printf("\taddiu\t$sp, $sp, %d\n", maxSize); // Adjust stack pointer
    printf("\tjr\t$31\n");

    fprintf(file, "\tmove\t$sp, $fp\n"); 
    fprintf(file, "\tlw\t$31, %d($sp)\n", maxSize - 4);
    fprintf(file, "\tlw\t$fp, %d($sp)\n", maxSize - 8);

    for (int i = 0; i < 8; i++) {
        fprintf(file, "\tlw\t$s%d, %d($sp)\t# Store $s%d at offset %d from the stack pointer\n", i, maxSize - 12 - 4 * i, i, maxSize - 12 - 4 * i);
    }

    fprintf(file, "\taddiu\t$sp, $sp, %d\n", maxSize);
    fprintf(file, "\tjr\t$31\n");


    fclose(file);
    return;
}

// Function to initialize arrays for parameters, locals, and globals
void initializeArrays() {
    // Initialize parameters array and their addresses
    for (int i = 0; i < MAX_PARAMS; i++) {
        params[i] = NULL;
        params_address[i] = 0;
    }

    // Initialize locals array and their addresses
    for (int i = 0; i < MAX_LOCALS; i++) {
        locals[i] = NULL;
        locals_address[i] = 0;
    }

    // Initialize globals array
    for (int i = 0; i < MAX_GLOBALS; i++) {
        globals[i] = NULL;
    }
    for (int i = 0; i < MAX_SAVED_REGISTERS; i++) {
        saved_registers[i] = NULL; // Initialize to NULL indicating unused
    }
}

void PrintMIPSFromThreeAddrInstrs(ThreeAddrInstr* instr_list) {



    initializeArrays();
    generateMIPSCode(instr_list);
    return 0;
}


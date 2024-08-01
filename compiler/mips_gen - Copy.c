
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"


extern ThreeAddrInstr* head;

int firstParam = 0;
int ParamCountSpecial;

// Assuming a maximum of 10 temporary variables for simplicity
#define MAX_VARS 10
char* varToRegMap[MAX_VARS]; // Maps variable names to register names
char* registers[] = { "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9" };
char* paramRegisters[] = { "$a0", "$a1", "$a2", "$a3" };
int ParamCount = 0;


#define MAX_PARAMS 8
char* paramTempRegMap[MAX_PARAMS]; // Maps parameter indices to temporary registers
char* tempRegisters[] = { "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7" }; // Temporary registers for parameters




void move(FILE* outputFile, char* regDest, char* regSource) {
    // Generate MIPS code for assignment
    if (regDest[0] != '$' && regSource[0] == '$') {
        // Destination is global, source is a register
        printf("sw %s, %s  # Assignment\n\n", regSource, regDest);
        fprintf(outputFile, "sw %s, %s  # Assignment\n\n", regSource, regDest);
    }
    else if (regDest[0] == '$' && regSource[0] != '$') {
        // Destination is a register, source is global
        printf("lw %s, %s  # Assignment\n\n", regDest, regSource);
        fprintf(outputFile, "lw %s, %s  # Assignment\n\n", regDest, regSource);
    }
    else {
        // Both are registers or both are global
        printf("move %s, %s  # Assignment\n\n", regDest, regSource);
        fprintf(outputFile, "move %s, %s  # Assignment\n\n", regDest, regSource);
    }
}




void generateDataSection(FILE* outputFile) {
    fprintf(outputFile, ".data\n");  // Start of the data section

    // Check if there is at least one scope (the global scope)
    if (currentScope >= 0) {
        SymbolTable* globalSymbolTable = &symbolTableStack[0];

        for (int i = 0; i < globalSymbolTable->size; i++) {
            if (globalSymbolTable->entries[i].type == VARIABLE) {
                // For each global variable, generate a line in the data section
                fprintf(outputFile, "%s: .space 4  # Global variable\n", globalSymbolTable->entries[i].name);
                printf("%s: .space 4  # Global variable\n", globalSymbolTable->entries[i].name);
            }
        }
    }

    fprintf(outputFile, "\n");  // End of the data section
    printf("\n");
}




char* GetTempRegisterForParam(int paramIndex) {
    if (paramIndex < 0 || paramIndex >= MAX_PARAMS) {
        printf("Error: Parameter index out of range\n");
        return NULL;
    }

    if (paramTempRegMap[paramIndex] == NULL) {
        // Assign a new temporary register to the parameter
        paramTempRegMap[paramIndex] = tempRegisters[paramIndex];
    }

    return paramTempRegMap[paramIndex];
}

int getTotalUsedRegisters() {
    // Count the number of used general-purpose registers
    int usedGeneralRegisters = 0;
    for (int i = 0; i < MAX_VARS; i++) {
        if (varToRegMap[i] != NULL) {
            usedGeneralRegisters++;
        }
    }

    // The total number of used registers is the sum of used general-purpose registers and used parameter registers
    int totalUsedRegisters = usedGeneralRegisters + 4;
    return totalUsedRegisters;
}

void storeCurrentRegisters(FILE* outputFile) {
    // Calculate the total number of registers to be saved
    int totalRegisters = getTotalUsedRegisters();

    // Adjust the stack pointer to allocate space for the saved registers
    fprintf(outputFile, "addi $sp, $sp, -%d  # Allocate space for saved registers\n", totalRegisters * 4);
    printf("addi $sp, $sp, -%d  # Allocate space for saved registers\n", totalRegisters * 4);

    // Save the registers on the stack
    for (int i = 0; i < MAX_VARS; i++) {
        if (varToRegMap[i] != NULL) {
            fprintf(outputFile, "sw %s, %d($sp)  # Save %s\n", registers[i], i * 4, registers[i]);
            printf("sw %s, %d($sp)  # Save %s\n", registers[i], i * 4, registers[i]);
        }
    }
    for (int i = 0; i < ParamCountSpecial; i++) {
        fprintf(outputFile, "sw %s, %d($sp)  # Save %s\n", paramRegisters[i], (MAX_VARS + i) * 4, paramRegisters[i]);
        printf("sw %s, %d($sp)  # Save %s\n", paramRegisters[i], (MAX_VARS + i) * 4, paramRegisters[i]);
    }
}

void restoreCurrentRegisters(FILE* outputFile) {
    // Restore the registers from the stack
    for (int i = 0; i < MAX_VARS; i++) {
        if (varToRegMap[i] != NULL) {
            fprintf(outputFile, "lw %s, %d($sp)  # Restore %s\n", registers[i], i * 4, registers[i]);
            printf("lw %s, %d($sp)  # Restore %s\n", registers[i], i * 4, registers[i]);
        }
    }
    for (int i = 0; i < ParamCountSpecial; i++) {
        fprintf(outputFile, "lw %s, %d($sp)  # Restore %s\n", paramRegisters[i], (MAX_VARS + i) * 4, paramRegisters[i]);
        printf("lw %s, %d($sp)  # Restore %s\n", paramRegisters[i], (MAX_VARS + i) * 4, paramRegisters[i]);
    }

    // Adjust the stack pointer to deallocate space for the saved registers
    int totalRegisters = getTotalUsedRegisters();
    fprintf(outputFile, "addi $sp, $sp, %d  # Deallocate space for saved registers\n", totalRegisters * 4);
    printf("addi $sp, $sp, %d  # Deallocate space for saved registers\n", totalRegisters * 4);
}




int main_flag = 0;

// Function to traverse the linked list and generate MIPS assembly
void PrintMIPSFromThreeAddrInstrs(ThreeAddrInstr* head) {
    ThreeAddrInstr* current = head;

    FILE* mipsFile = fopen("./MIPS_CODE.s", "a");
    if (!mipsFile) {
        perror("Error opening MIPS_CODE.s");
        return;
    }

    

    while (current != NULL) {
        GenerateMIPS(current, mipsFile);
        current = current->next;
    }
    GenerateMIPS(current, mipsFile);
    fclose(mipsFile);
}

// Function to find or assign a register to a variable
char* GetRegisterForVar(char* var) {
    // Check if the variable is global
    SymbolTable* globalSymbolTable = &symbolTableStack[0];
    for (int i = 0; i < globalSymbolTable->size; i++) {
        if (strcmp(globalSymbolTable->entries[i].name, var) == 0 && globalSymbolTable->entries[i].type == VARIABLE) {
            // Global variable found, return its name
            return var;
        }
    }

    for (int i = 0; i < MAX_VARS; i++) {
        if (varToRegMap[i] != NULL && strcmp(varToRegMap[i], var) == 0) {
            // Variable already mapped to a register
            return registers[i];
        }
    }

    // Assign a new register to the variable
    for (int i = 0; i < MAX_VARS; i++) {
        if (varToRegMap[i] == NULL) {
            varToRegMap[i] = var;
            return registers[i];
        }
    }

    // No available registers (this should be handled more gracefully)
    printf("Error: Out of registers\n");
    return NULL;
}

void GenerateMIPS(ThreeAddrInstr* instr, FILE* outputFile) {

    //generateDataSection(outputFile);

    if (!instr) {
        if (main_flag) {
            printf("li $v0, 10         # Load the exit system call code into $v0\nsyscall            # Execute the system call to exit\n\n");
            fprintf(outputFile, "li $v0, 10         # Load the exit system call code into $v0\nsyscall            # Execute the system call to exit\n\n");
            return;
            main_flag = 0;
        }
        else {
            printf("# Epilogue - restore $ra and deallocate stack space\n\nlw $ra, 0($sp)\naddi $sp, $sp, 4\n\njr $ra\n\n");
            fprintf(outputFile, "# Epilogue - restore $ra and deallocate stack space\n\nlw $ra, 0($sp)\naddi $sp, $sp, 4\n\njr $ra\n\n");
        }
        return;
    }
    switch (instr->op) {
    case OP_ASSIGN: {

        char* regDest = getParamRegister(instr->result); // Check if the destination is a parameter
        if (!regDest) {
            regDest = GetRegisterForVar(instr->result);
        }

        // Check if arg1 is numeric (integer constant)
        char* endptr;
        long val = strtol(instr->arg1, &endptr, 10);
        if (*endptr == '\0') { // Successfully converted to a number
            printf("# %s = %s\n\n", instr->result, instr->arg1);
            printf("li %s, %ld  # Load immediate value\n\n", regDest, val);
            fprintf(outputFile, "# %s = %s\n\n", instr->result, instr->arg1);
            fprintf(outputFile, "li %s, %ld  # Load immediate value\n\n", regDest, val);
        }
        else { // arg1 is not numeric, assume it's a variable

            char* regSource = getParamRegister(instr->arg1); // Check if the destination is a parameter
            if (!regSource) {
                regSource = GetRegisterForVar(instr->arg1);
            }

            //here

            move(outputFile, regDest, regSource);

            //    printf("# %s = %s\n\n", instr->result, instr->arg1);
            //    printf("move %s, %s  # Assignment\n\n", regDest, regSource);
            //    fprintf(outputFile, "# %s = %s\n\n", instr->result, instr->arg1);
            //    fprintf(outputFile, "move %s, %s  # Assignment\n\n", regDest, regSource);
            //}



            break;
        }
        break;
    }

    case OP_PARAM: {
        // Check if arg1 is numeric (integer constant)
        if (firstParam == 0) {
            storeCurrentRegisters(outputFile);
            firstParam = 1;
        }
        char* endptr;
        long val = strtol(instr->arg1, &endptr, 10);
        char* regTemp = GetTempRegisterForParam(ParamCount);

        if (*endptr == '\0') { // Successfully converted to a number, so it's a numeric constant



            printf("# param %s\n\n", instr->arg1);
            printf("li %s, %ld  # Load immediate value into argument register\n\n", regTemp, val);
            fprintf(outputFile, "# param %s\n\n", instr->arg1);
            fprintf(outputFile, "li %s, %ld  # Load immediate value into argument register\n\n", regTemp, val);
        }
        else { // arg1 is not numeric, assume it's a variable name

            char* regParam = getParamRegister(instr->arg1); // Check if the destination is a parameter
            if (!regParam) {
                regParam = GetRegisterForVar(instr->arg1);
            }


            move(outputFile, regTemp, regParam);

            printf("# param %s\n\n", instr->arg1);
            //printf("move %s, %s  # Move parameter into argument register\n\n", regTemp, regParam);
            fprintf(outputFile, "# param %s\n\n", instr->arg1);
            //fprintf(outputFile, "move %s, %s  # Move parameter into argument register\n\n", regTemp, regParam);
        }
        ParamCount++;
        break;
    }

    case CALL: {
        if (firstParam == 0) {
            storeCurrentRegisters(outputFile);
            firstParam = 1;
        }
        for (int i = 0; i < ParamCount; i++) {
            char* regTemp = GetTempRegisterForParam(i);

            char* regDest = (char*)malloc(sizeof(char) * 10); // Allocate memory for the register name
            sprintf(regDest, "$a%d", i);

            move(outputFile, regDest, regTemp);

            //printf("move $a%d, %s  # Move parameter to argument register\n\n", i, regTemp);
            //fprintf(outputFile, "move $a%d, %s  # Move parameter to argument register\n\n", i, regTemp);
        }

        ParamCount = 0;
        printf("# call %s\n\n", instr->arg1);
        printf("jal %s  # Jump to function\n\n", instr->arg1);
        fprintf(outputFile, "# call %s\n\n", instr->arg1);
        fprintf(outputFile, "jal %s  # Jump to function\n\n", instr->arg1);


        if (firstParam == 1) {
            restoreCurrentRegisters(outputFile);
            firstParam = 0;
        }

        break;
    }
    case OP_FUNC_DEF: {

        for (int i = 0; i < MAX_VARS; i++) {
            varToRegMap[i] = NULL;
        }

        if (strcmp("main", instr->result) == 0) {
            main_flag = 1;
        }
        // Label the start of the function
        printf("\n%s:\n", instr->result); // Use result field for function name
        // Assuming all functions save return address ($ra) and set up stack space if needed
        printf("# Prologue - save $ra and allocate stack space if necessary\n\n");
        printf("addi $sp, $sp, -4  # Adjust stack pointer to save $ra\n");
        printf("sw $ra, 0($sp)  # Save return address on the stack\n\n");

        // If local variables or parameters need stack space, allocate it here

        printf("    # Function body begins\n\n");

        fprintf(outputFile, "\n%s:\n", instr->result); // Use result field for function name
        // Assuming all functions save return address ($ra) and set up stack space if needed
        fprintf(outputFile, "# Prologue - save $ra and allocate stack space if necessary\n\n");
        fprintf(outputFile, "addi $sp, $sp, -4  # Adjust stack pointer to save $ra\n");
        fprintf(outputFile, "sw $ra, 0($sp)  # Save return address on the stack\n\n");

        // If local variables or parameters need stack space, allocate it here

        fprintf(outputFile, "    # Function body begins\n\n");
        ParamCountSpecial = instr->ArgCount;
        break;
    }

                    // Add other cases here
    default:
        printf("# Unsupported OpCode\n\n");
        fprintf(outputFile, "# Unsupported OpCode\n\n");
    }

}

void printFileContents() {
    printf(".align 2\n"
        ".data\n"
        "_nl: .asciiz \"\\n\"\n"
        ".align 2\n"
        ".text\n"
        "# println: print out an integer followed by a newline\n"
        "println:\n"
        "li $v0, 1          # System call for print integer\n"
        "syscall            # Execute syscall (prints the integer in $a0)\n\n"
        "li $v0, 4          # System call for print string\n"
        "la $a0, _nl        # Load address of newline character into $a0\n"
        "syscall            # Execute syscall (prints newline)\n\n"
        "jr $ra             # Return to caller\n");
}

void appendFileContentsToFile(FILE* outputFile) {
    fprintf(outputFile, ".align 2\n"
        ".data\n"
        "_nl: .asciiz \"\\n\"\n"
        ".align 2\n"
        ".text\n"
        "# println: print out an integer followed by a newline\n"
        "println:\n"
        "li $v0, 1          # System call for print integer\n"
        "syscall            # Execute syscall (prints the integer in $a0)\n\n"
        "li $v0, 4          # System call for print string\n"
        "la $a0, _nl        # Load address of newline character into $a0\n"
        "syscall            # Execute syscall (prints newline)\n\n"
        "jr $ra             # Return to caller\n");
}


char* currentFuncParams[4]; // Assuming a maximum of 4 parameters for simplicity

// Function to set the parameters for the current function
void setCurrentFuncParams(FuncDefNode* funcDefNode) {
    // Reset the current function parameters
    for (int i = 0; i < 4; i++) {
        currentFuncParams[i] = NULL;
    }

    // Set the parameters for the current function
    for (int i = 0; i < funcDefNode->nargs && i < 4; i++) {
        currentFuncParams[i] = funcDefNode->argnames[i];
    }
}

// Function to get the register corresponding to a function parameter
char* getParamRegister(char* paramName) {
    for (int i = 0; i < 4; i++) {
        if (currentFuncParams[i] && strcmp(currentFuncParams[i], paramName) == 0) {
            return paramRegisters[i]; // 'registers' array is defined in your MIPS generation code
        }
    }
    return NULL;
}
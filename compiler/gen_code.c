
#include "gen_code.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
TO DO:
1) FIX RETURN
2) FIX BOOL
3) IMPLEMENT CLEAN FOR $sv0
4) IMPLEMENT CODE GENERATION MIPS
5) IMPLEMENT $sv# SAVING
*/










int tempCounter;
// Global variable to keep track of the label count
static int labelCount = 2;
// Global pointers for the linked list
ThreeAddrInstr* head = NULL;
ThreeAddrInstr* lastInstruction = NULL;
extern int gen_3AC_flag;

void AppendInstruction(ThreeAddrInstr* instr) {
    if (!instr) return; // Safety check

    if (instr->op == OP_FUNC_DEF) {
        instr->next = head;
        head = instr;
        return;
    }

    if (head == NULL) {
        // If the list is empty, this instruction is the new head
        head = instr;
        lastInstruction = instr;
    }
    else {
        // Otherwise, append it to the end of the list
        lastInstruction->next = instr;
    }

    // Update lastInstruction to point to the new last instruction
    lastInstruction = instr;
}

// Function to generate a new unique label
char* generateLabel() {
    // Allocate memory for the label string
    char* label = (char*)malloc(20 * sizeof(char));
    if (!label) {
        fprintf(stderr, "Memory allocation failed for label\n");
        exit(EXIT_FAILURE);
    }

    // Generate the label string with a unique number
    snprintf(label, 20, "$L%d", labelCount);

    // Increment the label count for the next call
    labelCount++;

    return label;
}


ThreeAddrInstr* newInstruction(OpCode op, const char* arg1, const char* arg2, const char* result) {
    ThreeAddrInstr* instr = (ThreeAddrInstr*)malloc(sizeof(ThreeAddrInstr));
    instr->op = op;
    instr->arg1 = arg1 ? strdup(arg1) : NULL;
    instr->arg2 = arg2 ? strdup(arg2) : NULL;
    instr->result = result ? strdup(result) : NULL;
    instr->next = NULL;
    AppendInstruction(instr);
    return instr;
}


IdentifierNode* createIdentifierNode(const char* name) {
    IdentifierNode* node = (IdentifierNode*)malloc(sizeof(IdentifierNode));
    node->type = IDENTIFIER;
    node->name = strdup(name); // Ensure the string is duplicated to manage memory correctly
    return node;
}

char* generateArithHolder() {
    char* buffer = (char*)malloc(20 * sizeof(char));
    snprintf(buffer, 20, "__temp__%d", tempCounter);
    tempCounter++;
    return buffer;
}

char* arith_instr(void* astNode, ThreeAddrInstr** lastInstruction) {
    if (!astNode) return;

    NodeType type = ast_node_type(astNode);



    // Variables to hold the parts of an instruction if needed
    char* arg1 = NULL, * arg2 = NULL, * result = NULL;
    ThreeAddrInstr* instr = NULL;
    char argBuff[50] = { 0 };

    // Handle different types of nodes
    switch (type) {
    case ADD:
    case SUB:
    case MUL:
    case DIV: {
        // Binary expression handling
        BinaryExprNode* binaryNode = (BinaryExprNode*)astNode;
        OpCode opCode = (type == ADD) ? OP_ADD : (type == SUB) ? OP_SUB : (type == MUL) ? OP_MUL : OP_DIV;
        arg1 = strdup(arith_instr(binaryNode->operand1, lastInstruction));
        arg2 = strdup(arith_instr(binaryNode->operand2, lastInstruction));
        
        
        result = generateArithHolder();
        instr = newInstruction(opCode, arg1, arg2, result);
        return result;
        break;
    }
    case UMINUS: {
        // Unary minus handling
        UnaryExprNode* unaryNode = (UnaryExprNode*)astNode;
        
        arg1 = strdup(arith_instr(unaryNode->operand, lastInstruction)); // Assuming operand returns a name after processing
        result = generateArithHolder(); // Function to generate a temporary variable
        instr = newInstruction(OP_UMINUS, arg1, NULL, result);
        return result;
        break;
    }
    case FUNC_CALL: {
        FuncCallNode* callNode = (FuncCallNode*)astNode;

        // Process each argument in the list
        ExprListNode* args = (ExprListNode*)func_call_args(callNode);
        int argIndex = 0;
        while (args) {
            // Generate an instruction to move the argument into the temporary variable
            newInstruction(OP_PARAM, arith_instr(args, lastInstruction), NULL, NULL);

            args = args->next; // Next argument
            argIndex++;
        }

        char argCount[50] = { 0 };
        snprintf(argCount, sizeof(argCount), "%d", argIndex);

        // After processing arguments, generate the actual call instruction
        instr = newInstruction(CALL, func_call_callee(callNode), argCount, NULL);
        result = generateArithHolder();
        instr = newInstruction(OP_RETURN_CALLER, NULL, NULL, result);
        return result;
        break;
    }
    case IDENTIFIER: {
        IdentifierNode* idNode = (IdentifierNode*)astNode;
        char* tempVarName = generateArithHolder(); 
        instr = newInstruction(OP_ASSIGN, idNode->name, NULL, tempVarName);
        return tempVarName;
        break;
    }

    case INTCONST: {
        IntConstNode* intConstNode = (IntConstNode*)astNode;
        char intConstValue[20];
        snprintf(intConstValue, sizeof(intConstValue), "%d", intConstNode->value);
        char* tempVarName = generateArithHolder(); 
        instr = newInstruction(OP_ASSIGN, intConstValue, NULL, tempVarName);
        return tempVarName;
        break;
    }
    case EXPR_LIST: {
        // Cast the astNode to ExprListNode* first
        ExprListNode* exprList = (ExprListNode*)astNode;
        // Now pass the expression part of the first node in the expression list
        return arith_instr(exprList->expr, lastInstruction);
    }

    default:
        // If the node type is not handled, return without action
        return NULL;
    }


    return NULL;
    // Update the last instruction pointer
    //*lastInstruction = instr;
}

char* bool_Instr(void* astNode) {
    if (!astNode) return NULL;

    char* result = generateArithHolder();
    NodeType type = ast_node_type(astNode);


    switch (type) {

    case EQ: {
        BinaryExprNode* expr = (BinaryExprNode*)astNode;
        char* arg1 = arith_instr(expr->operand1, lastInstruction);
        char* arg2 = arith_instr(expr->operand2, lastInstruction);
        newInstruction(OP_EQ, arg1, arg2, result);
        break;
    }

    case NE: {
        BinaryExprNode* expr = (BinaryExprNode*)astNode;

        newInstruction(OP_NE, arith_instr(expr->operand1, lastInstruction), arith_instr(expr->operand2, lastInstruction), result);
        break;
    }
    case LE: {
        BinaryExprNode* expr = (BinaryExprNode*)astNode;

        newInstruction(OP_LE, arith_instr(expr->operand1, lastInstruction), arith_instr(expr->operand2, lastInstruction), result);
        break;
    }
    case LT: {
        BinaryExprNode* expr = (BinaryExprNode*)astNode;

        newInstruction(OP_LT, arith_instr(expr->operand1, lastInstruction), arith_instr(expr->operand2, lastInstruction), result);
        break;
    }
    case GE: {
        BinaryExprNode* expr = (BinaryExprNode*)astNode;

        newInstruction(OP_GE, arith_instr(expr->operand1, lastInstruction), arith_instr(expr->operand2, lastInstruction), result);
        break;
    }
    case GT: {
        BinaryExprNode* expr = (BinaryExprNode*)astNode;


        newInstruction(OP_GT, arith_instr(expr->operand1, lastInstruction), arith_instr(expr->operand2, lastInstruction), result);
        break;
    }
    case AND: {
        BinaryExprNode* expr = (BinaryExprNode*)astNode;

        newInstruction(OP_AND, bool_Instr(expr->operand1), bool_Instr(expr->operand2), result);
        break;
    }
    case OR: {
        BinaryExprNode* expr = (BinaryExprNode*)astNode;

        newInstruction(OP_OR, bool_Instr(expr->operand1), bool_Instr(expr->operand2), result);
        break;
    }

        break;
    default:
        printf("Unhandled AST node type: %d\n", type);
        break;
    }
    return result;
}

// Wrapper function to initialize the context and start instruction generation
ThreeAddrInstr* generateInstructions(void* astNode) {
    ThreeAddrInstr* firstInstr = NULL;
    ThreeAddrInstr* lastInstruction = NULL; 
    firstInstr = generateInstructionsRec(astNode, &lastInstruction);
    return firstInstr;
}

ThreeAddrInstr* generateInstructionsRec(void* astNode, ThreeAddrInstr** lastInstruction) {
    if (!astNode) return NULL;

    NodeType type = ast_node_type(astNode);
    ThreeAddrInstr* instr = NULL, * argInstr = NULL, * firstInstr = NULL;
    char result[20] = { 0 }, argBuff[50] = { 0 };
    char* arg1, * arg2;

    switch (type) {

    case FUNC_DEF: {
        FuncDefNode* defNode = (FuncDefNode*)astNode;
        //setCurrentFuncParams(defNode); // Set the current function parameters

        char result[20];
        snprintf(result, sizeof(result), "%s", defNode->name);
        instr = newInstruction(OP_FUNC_DEF, NULL, NULL, result);
        instr->ArgCount = defNode->nargs;
        instr->argnames = defNode->argnames;
        break;
    }

    case FUNC_CALL: {
        FuncCallNode* callNode = (FuncCallNode*)astNode;

        // Process each argument in the list
        ExprListNode* args = (ExprListNode*)func_call_args(callNode);
        int argIndex = 0;
        while (args) {
            // Generate an instruction to move the argument into the temporary variable
            newInstruction(OP_PARAM, arith_instr(args, lastInstruction), NULL, NULL);

            args = args->next; // Next argument
            argIndex++;
        }

        char argCount[50] = { 0 };
        snprintf(argCount, sizeof(argCount), "%d", argIndex);

        // After processing arguments, generate the actual call instruction
        instr = newInstruction(CALL, func_call_callee(callNode), argCount, NULL);
        break;
    }
    case ASSG: {
        AssgNode* assgNode = (AssgNode*)astNode;

        // Left-hand side is directly used; no simplification expected for LHS
        char* lhsVarName = stmt_assg_lhs(assgNode);

 
        char* rhsVarName = arith_instr(assgNode->rhs, lastInstruction);

        // Generate the assignment instruction
        instr = newInstruction(OP_ASSIGN, rhsVarName, NULL, lhsVarName);
        break;
    }

    case IDENTIFIER: {
        IdentifierNode* idNode = (IdentifierNode*)astNode;


        char *tempVarName = generateTmpVar();

        // Generate a 3-address instruction that moves the identifier's value to the temp variable
        instr = newInstruction(OP_ASSIGN, idNode->name, NULL, tempVarName);
        // Mark the identifier node as replaced with a temporary variable
        idNode->replaced = 1;
        idNode->replacedWith = createIdentifierNode(tempVarName);

        break;
    }

    case IF: {
        IfNode* ifNode = (IfNode*)astNode;
        char* exprResult = bool_Instr(ifNode->expr);

        char* elseLabel = (ifNode->else_branch) ? generateLabel() : NULL;
        char* endLabel = generateLabel();

        if (ifNode->else_branch) {
            lastInstruction = newInstruction(OP_IF_GOTO, exprResult, elseLabel, NULL);
            traverseAndComposeInstructions(ifNode->then_branch, *lastInstruction);
            lastInstruction = newInstruction(OP_GOTO, NULL, NULL, endLabel);
            lastInstruction = newInstruction(OP_LABEL, NULL, NULL, elseLabel);
            traverseAndComposeInstructions(ifNode->else_branch, *lastInstruction);
            lastInstruction = newInstruction(OP_LABEL, NULL, NULL, endLabel);
        }
        else {
            lastInstruction = newInstruction(OP_IF_GOTO, exprResult, endLabel, NULL);
            traverseAndComposeInstructions(ifNode->then_branch, *lastInstruction);
            lastInstruction = newInstruction(OP_LABEL, NULL, NULL, endLabel);
        }

        break;
    }

    case WHILE: {
        WhileNode* whileNode = (WhileNode*)astNode;
        char* exprLabel = generateLabel();
        char* bodyLabel = generateLabel();

        lastInstruction = newInstruction(OP_GOTO, NULL, NULL, exprLabel);
        lastInstruction = newInstruction(OP_LABEL, NULL, NULL, bodyLabel);
        traverseAndComposeInstructions(whileNode->body, *lastInstruction);

        lastInstruction = newInstruction(OP_LABEL, NULL, NULL, exprLabel);

        char* temp = bool_Instr(whileNode->expr);

        //temp->ArgCount = 1;
       // lastInstruction = temp;

        lastInstruction = newInstruction(OP_IF_GOTO, temp, NULL, bodyLabel);
        break;
    }
    case RETURN: {
        ReturnNode* rtrnNode = (ReturnNode*)astNode;
        lastInstruction = newInstruction(OP_RETURN, arith_instr(rtrnNode->expr, lastInstruction), NULL, NULL);
        break;
    }

    default:
        printf("Unhandled AST node type: %d\n", type);
        break;
    }
    return instr;
}

char* generateTmpVar() {
    static char buffer[20];
    snprintf(buffer, sizeof(buffer), "__reg__");
    return buffer;
}


void traverseAndComposeInstructions(void* astNode, ThreeAddrInstr** lastInstruction) {
    if (!astNode) return;

    // Cast to DummyNode to check for the 'replaced' flag.

    NodeType nodeType = ast_node_type(astNode);
    switch (nodeType) {
    case FUNC_DEF: {
        // Traverse function body
        FuncDefNode* funcDefNode = (FuncDefNode*)astNode;
        traverseAndComposeInstructions(funcDefNode->body, lastInstruction);
        break;
    }
    case IF: {

        IfNode* ifNode = (IfNode*)astNode;
        //traverseAndComposeInstructions(ifNode->expr, lastInstruction);
        //traverseAndComposeInstructions(ifNode->then_branch, lastInstruction);
        //if (ifNode->else_branch) {
        //    traverseAndComposeInstructions(ifNode->else_branch, lastInstruction);
        //}
        break;
    }
    case WHILE: {
        // Traverse WHILE condition and body
        WhileNode* whileNode = (WhileNode*)astNode;
        //traverseAndComposeInstructions(whileNode->expr, lastInstruction);
        //traverseAndComposeInstructions(whileNode->body, lastInstruction);
        break;
    }
    case STMT_LIST:
    case EXPR_LIST: {
        // Traverse statement or expression list
        StmtListNode* currentNode = (StmtListNode*)astNode;
        while (currentNode) {
            traverseAndComposeInstructions(currentNode->stmt, lastInstruction);
            currentNode = (StmtListNode*)currentNode->next;
        }
        return;
        break;
    }
    case ASSG: {

        AssgNode* assgNode = (AssgNode*)astNode;
        
        break;
    }
    case RETURN: {

    }
    default:
        //printf("Unhandled AST Node Type\n");
        break;
    }

    // After traversing child nodes, generate instructions for the current node.
    generateInstructionsRec(astNode, lastInstruction);
}

// Entry point to start the traversal and instruction generation process.
ThreeAddrInstr* startInstructionGeneration(void* root) {
    ThreeAddrInstr* firstInstr = NULL;
    traverseAndComposeInstructions(root, &firstInstr);
    return firstInstr; // The generated instructions are linked starting from firstInstr.
}


// Function to print a single 3AC instruction
void print3ACInstruction(ThreeAddrInstr* instr) {
    if (!instr) return;

    // Determine the operation type and format the instruction accordingly
    char opStr[20] = { 0 };
    switch (instr->op) {
    case OP_ADD: strcpy(opStr, "+"); break;
    case OP_SUB: strcpy(opStr, "-"); break;
    case OP_MUL: strcpy(opStr, "*"); break;
    case OP_DIV: strcpy(opStr, "/"); break;

    case OP_EQ: strcpy(opStr, "=="); break;
    case OP_NE: strcpy(opStr, "!="); break;
    case OP_LT: strcpy(opStr, "<"); break;
    case OP_LE: strcpy(opStr, "<="); break;
    case OP_GT: strcpy(opStr, ">"); break;
    case OP_GE: strcpy(opStr, ">="); break;

    case OP_AND: strcpy(opStr, "AND"); break;
    case OP_OR: strcpy(opStr, "OR"); break;

    case CALL: strcpy(opStr, "call"); break;
    case OP_ASSIGN: strcpy(opStr, "="); break;
    case OP_PARAM: strcpy(opStr, "param"); break;

    default: strcpy(opStr, "UNKNOWN"); break;
    }

    if (gen_3AC_flag) {

        if (instr->op == CALL) {
            printf("%s %s, %s\n", opStr, instr->arg1, instr->arg2);
        }
        else if (instr->op == OP_ASSIGN) {
            printf("%s = %s\n", instr->result, instr->arg1);
        }
        else if (instr->op == OP_PARAM) {
            printf("%s %s\n", opStr, instr->arg1);
        }
        else if (instr->op == OP_UMINUS) { // For unary operations and assignments
            printf("%s = - %s\n", instr->result, instr->arg1);
        }
        else if (instr->op == OP_RETURN) {
            printf("Return %s\n", instr->arg1);
        }
        else if (instr->op == OP_RETURN_CALLER) {
            printf("Retrieve %s = CALL\n", instr->result);
        }
        else if (instr->op == OP_ADD || instr->op == OP_SUB || instr->op == OP_MUL || instr->op == OP_DIV || instr->op == OP_AND || instr->op == OP_OR || instr->op == OP_EQ || instr->op == OP_NE || instr->op == OP_LE ||
            instr->op == OP_LT || instr->op == OP_GE || instr->op == OP_GT ||
            instr->op == OP_AND || instr->op == OP_OR) {


            printf("%s = (%s %s %s)\n", instr->result, instr->arg1, opStr, instr->arg2);

        }
        else if (instr->op == OP_IF_GOTO) {
            if (instr->arg2) {
                printf("NEGATIVE IF GOTO %s\n", instr->arg2);
            }
            else {
                printf("POSITIVE IF GOTO %s\n", instr->result);
            }
        }
        else { // For labels and jumps
            printf("%s\n", instr->result);
        }
    }

    FILE* file = fopen("./instr.txt", "a");  // Open the file for writing, overwriting existing contents

    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    else if (instr->op == CALL) {
        fprintf(file, "%s %s, %s\n", opStr, instr->arg1, instr->arg2);
    }
    else if (instr->op == OP_ASSIGN) {
        fprintf(file, "%s = %s\n", instr->result, instr->arg1);
    }
    else if (instr->op == OP_PARAM) {
        fprintf(file, "%s %s\n", opStr, instr->arg1);
    }
    else if (instr->op == OP_UMINUS) { // For unary operations and assignments
        fprintf(file, "%s = - %s\n", instr->result, instr->arg1);
    }
    else if (instr->op == OP_RETURN) {
        fprintf(file, "Return %s\n", instr->arg1);
    }
    else if (instr->op == OP_RETURN_CALLER) {
        fprintf(file, "Retrieve %s = CALL\n", instr->result);
    }
    else if (instr->op == OP_EQ || instr->op == OP_NE || instr->op == OP_LE ||
        instr->op == OP_LT || instr->op == OP_GE || instr->op == OP_GT ||
        instr->op == OP_AND || instr->op == OP_OR) { 


         fprintf(file, "%s = (%s %s %s)\n", instr->result, instr->arg1, opStr, instr->arg2);

    }
    else if (instr->op == OP_IF_GOTO) {
        if (instr->arg2) {
            fprintf(file, "NEGATIVE IF GOTO %s\n", instr->arg2);
        }
        else {
            fprintf(file, "POSITIVE IF GOTO %s\n", instr->result);
        }
    }
    else { // For labels and jumps
        fprintf(file, "%s\n", instr->result);
    }

    fclose(file);  


}

// Function to traverse the AST, generate 3AC instructions, and print them
void print3ACInstructionsFromAST(void* rootAST) {

    startInstructionGeneration(rootAST);
    ThreeAddrInstr* currentInstr = head;

    //printf("Generated Three-Address Code (3AC) Instructions:\n");
    while (currentInstr != NULL) {
        print3ACInstruction(currentInstr);
        currentInstr = currentInstr->next;
    }



    PrintMIPSFromThreeAddrInstrs(head);

    // Freeing the allocated instructions and strings
    ThreeAddrInstr* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        if (temp->arg1) free(temp->arg1);
        if (temp->arg2) free(temp->arg2);
        free(temp);
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



void generateDataSection(FILE* outputFile) {
    fprintf(outputFile, ".data\n");  // Start of the data section

    // Check if there is at least one scope (the global scope)
    if (currentScope >= 0) {
        SymbolTable* globalSymbolTable = &symbolTableStack[0];

        for (int i = 0; i < globalSymbolTable->size; i++) {
            if (globalSymbolTable->entries[i].type == VARIABLE) {
                // For each global variable, generate a line in the data section
                fprintf(outputFile, "_1_%s_1_: .space 4  # Global variable\n", globalSymbolTable->entries[i].name);
                printf("_1_%s_1_: .space 4  # Global variable\n", globalSymbolTable->entries[i].name);
            }
        }
    }

    fprintf(outputFile, "\n");  // End of the data section
    printf("\n");
}

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "scanner.h"
#include "ast.h"
#include "gen_code.h"


extern int print_ast_flag; // Declared in driver.c
extern int gen_code_flag;
extern int get_token();
extern char* lexeme;
extern int line_number;
extern int add_to_symbol_table(char* name, IdType type, Scope scope);
extern int check_function_call(char* name, int argCount);
extern int check_variable_declaration(char* name);
extern void print3ACInstruction(ThreeAddrInstr* instr);

char* prelexeme;

int lookahead;
int prelookahead;

int counter = 0;


void advance() {
    if (counter != 0) {
        prelexeme = strdup(lexeme);
    }
    counter++;
    prelookahead = lookahead;
    lookahead = get_token();
}

void match(int expectedToken) {
    if (lookahead == EOF) {
        fprintf(stderr, "ERROR: Input ended unexpectedly. Incomplete statement at EOF. on LINE %d\n", line_number);
        exit(1);
    }
    else if (lookahead == expectedToken) {
        advance();
    }
    else {
        if (lexeme != NULL) {
            fprintf(stderr, "ERROR: Unexpected token '%s' on LINE %d\n", lexeme, line_number);
        }
        else {
            fprintf(stderr, "ERROR: Unexpected end of input on LINE %d\n", line_number);
        }
        exit(1);
    }
}

int parse() {
    advance();
    prog();
    if (lookahead != EOF) {
        fprintf(stderr, "ERROR: Extra code after program end on LINE %d\n", line_number);
        return 1;
    }
    return 0;
}

void prog() {
    while (lookahead == kwINT) {
        decl_or_func();
    }
    if (lookahead != EOF) {
        fprintf(stderr, "ERROR: Expected function definition, variable declaration, or end of input, found '%s' on LINE %d\n", lexeme, line_number);
        exit(1);
    }
}

void decl_or_func() {
    type();
    char* id_name = strdup(lexeme);
    match(ID);
    if (lookahead == LPAREN) {
        if (add_to_symbol_table(id_name, FUNCTION, GLOBAL) == -1) {
            fprintf(stderr, "ERROR: Function '%s' redeclared on LINE %d\n", id_name, line_number);
            exit(1);
        }
        match(LPAREN);
        push_scope();
        char** arg_names = NULL;
        int arg_count = opt_formals(&arg_names);

        update_function_arg_count(id_name, arg_count);
        match(RPAREN);

        match(LBRACE);

        // Initialize an empty list for function body statements
        StmtListNode* body = NULL;

        opt_var_decls();
        body = opt_stmt_list();


        if (lookahead == RBRACE) {
            // Create the function definition AST node
            FuncDefNode* func_def = create_func_def_node(id_name, arg_count, arg_names, body);

            // Print the AST if the flag is set
            if (print_ast_flag) {
                print_ast(func_def);
            }
            if (gen_code_flag) {
                print3ACInstructionsFromAST(func_def);
            }

        }
        match(RBRACE);
        pop_scope();

    }
    else {
        if (add_to_symbol_table(id_name, VARIABLE, GLOBAL) == -1) {
            fprintf(stderr, "ERROR: Global variable '%s' redeclared on LINE %d\n", id_name, line_number);
            exit(1);
        }
        id_list();
        match(SEMI);
    }
    free(id_name);
}


void id_list() {
    while (lookahead == COMMA) {
        match(COMMA);
        char* id_name = strdup(lexeme);
        match(ID);
        if (add_to_symbol_table(id_name, VARIABLE, GLOBAL) == -1) {
            fprintf(stderr, "ERROR: Global variable '%s' redeclared on LINE %d\n", id_name, line_number);
            exit(1);
        }
        free(id_name);
    }
}

void type() {
    match(kwINT);
}

int opt_formals(char*** arg_names) {
    int arg_count = 0;
    if (lookahead == kwINT) {
        arg_count = formals(arg_names);
    }
    return arg_count;
}

int formals(char*** arg_names) {
    int arg_count = 0;
    *arg_names = NULL;

    while (lookahead == kwINT) {
        type();
        char* id_name = strdup(lexeme);
        match(ID);
        if (add_to_symbol_table(id_name, VARIABLE, LOCAL) == -1) {
            fprintf(stderr, "ERROR: Formal parameter '%s' redeclared on LINE %d\n", id_name, line_number);
            exit(1);
        }

        arg_count++;
        char** temp = realloc(*arg_names, arg_count * sizeof(char*));
        if (!temp) {
            // Handle allocation failure gracefully
            fprintf(stderr, "Error reallocating memory\n");
            return -1;
        }
        *arg_names = temp;
        (*arg_names)[arg_count - 1] = id_name;

        if (lookahead != COMMA) {
            break;
        }
        match(COMMA);
    }

    return arg_count;
}



void opt_var_decls() {
    if (lookahead == kwINT) {
        decl_or_func();
        opt_var_decls();
    }
}

StmtListNode* opt_stmt_list() {
    StmtListNode* head = NULL, * tail = NULL;

    while (lookahead == ID || lookahead == kwWHILE || lookahead == kwIF || lookahead == LBRACE || lookahead == SEMI || lookahead == kwRETURN) {
        void* stmt_node = stmt();
        StmtListNode* new_node = create_stmt_list_node(stmt_node, NULL);

        if (!head) {
            head = new_node;
            tail = new_node;
        }
        else {
            tail->next = new_node;
            tail = new_node;
        }
    }

    return head;
}

void* stmt() {
    void* node = NULL;

    if (lookahead == ID) {
        match(ID); ///////
        node = fn_call_or_assg();
        match(SEMI);
    }
    else if (lookahead == kwWHILE) {
        node = while_stmt();
    }
    else if (lookahead == kwIF) {
        node = if_stmt();
    }
    else if (lookahead == LBRACE) {
        match(LBRACE);
        node = opt_stmt_list();
        match(RBRACE);
    }
    else if (lookahead == SEMI) {
        match(SEMI);
    }
    else if (lookahead == kwRETURN) {
        node = return_stmt();
    }
    else {
        fprintf(stderr, "ERROR: Invalid statement on LINE %d\n", line_number);
        exit(1);
    }

    return node; // Return the pointer to the specific statement node
}




IfNode* if_stmt() {
    match(kwIF);
    match(LPAREN);
    void* expr = bool_exp(); // Capture the boolean expression
    match(RPAREN);
    void* then_branch = stmt(); // Capture the then branch

    void* else_branch = NULL; // Initialize the else branch as NULL
    if (lookahead == kwELSE) {
        match(kwELSE);
        else_branch = stmt(); // Capture the else branch if it exists
    }

    return create_if_node(expr, then_branch, else_branch); // Create and return an IfNode
}


WhileNode* while_stmt() {
    match(kwWHILE);
    match(LPAREN);
    void* expr = bool_exp();
    match(RPAREN);
    void* body = stmt();

    return create_while_node(expr, body); // Use the helper function to create and return a WhileNode
}


ReturnNode* return_stmt() {
    match(kwRETURN);
    void* expr = NULL; // Initialize expression as NULL for cases with no return value
    if (lookahead != SEMI) {
        expr = arith_exp(); // Capture the return expression
    }
    match(SEMI);

    return create_return_node(expr); // Create and return a ReturnNode
}


void* fn_call_or_assg() {
    char* id_name = strdup(prelexeme);
    //match(ID);

    if (lookahead == opASSG) {
        // Handle assignment
        if (check_variable_declaration(id_name) == -1) {
            fprintf(stderr, "ERROR: Use of undeclared variable '%s' on LINE %d\n", id_name, line_number);
            exit(1);
        }
        match(opASSG);
        void* expr_node = arith_exp();
        AssgNode* assg_node = create_assg_node(id_name, expr_node);
        return assg_node;
    }
    else {
        // Handle function call
        match(LPAREN);
        int argCount = 0;
        ExprListNode* args = NULL;
        if (lookahead != RPAREN) {
            args = expr_list(&argCount);
        }
        match(RPAREN);

        switch (check_function_call(id_name, argCount)) {
        case -1:
            fprintf(stderr, "ERROR: Call to undeclared function '%s' on LINE %d\n", id_name, line_number);
            free(id_name);
            exit(1);
            return NULL;
            break;
        case -2:
            fprintf(stderr, "ERROR: Call to function '%s' on LINE %d with incorrect number of arguments\n", id_name, line_number);
            free(id_name);
            exit(1);
            return NULL;
            break;
        case -3:
            fprintf(stderr, "ERROR: '%s' on LINE %d, is of the wrong type, should be a variable.\n", id_name, line_number);
            free(id_name);
            exit(1);
            return NULL;
            break;
        default:
            break;
        }

        if (check_function_call(id_name, argCount) == -2) {
            fprintf(stderr, "ERROR: Call to function '%s' on LINE %d with incorrect number of arguments\n", id_name, line_number);
            free(id_name);
            exit(1);
            return NULL;
        }
        if (check_function_call(id_name, argCount) == -1) {
            fprintf(stderr, "ERROR: Call to undeclared function '%s' on LINE %d\n", id_name, line_number);
            free(id_name);
            exit(1);
            return NULL;
        }

        //if (!is_function_arg_count_correct(id_name, argCount)) {
        //    fprintf(stderr, "ERROR: Call to function '%s' on LINE %d with incorrect number of arguments\n", id_name, line_number);
        //    free(id_name);
        //    return NULL;
        // }

        FuncCallNode* funcCallNode = create_func_call_node(id_name, args);
        return funcCallNode;
    }
}




ExprListNode* expr_list(int* arg_count) {
    ExprListNode* head = NULL, * tail = NULL;
    *arg_count = 0;  // Initialize the count of arguments

    do {
        DummyNode* expr_node = (DummyNode*)arith_exp();



        if (expr_node) {
            ExprListNode* new_node = create_expr_list_node(expr_node, NULL);
            if (!head) {
                head = new_node;
            }
            else {
                tail->next = new_node;
            }
            tail = new_node;
            (*arg_count)++;  // Increment the count for each argument
        }

        if (lookahead != COMMA) break;  // Exit loop if no more arguments
        match(COMMA);
    } while (true);

    return head;
}

BinaryExprNode* bool_exp() {
    BinaryExprNode* node = and_exp();

    while (lookahead == opOR) {
        match(opOR);
        BinaryExprNode* rightNode = and_exp();
        node = create_binary_expr_node(OR, node, rightNode);
    }
    return node;
}

BinaryExprNode* and_exp() {
    BinaryExprNode* node = relational_exp();

    while (lookahead == opAND) {
        match(opAND);
        BinaryExprNode* rightNode = relational_exp();
        node = create_binary_expr_node(AND, node, rightNode);
    }
    return node;
}

BinaryExprNode* relational_exp() {
    void* operand1 = arith_exp();
    NodeType relopType = relop();
    void* operand2 = arith_exp();

    return create_binary_expr_node(relopType, operand1, operand2);
}


void* arith_exp() {
    void* node = term();
    while (lookahead == opADD || lookahead == opSUB) {
        NodeType operation = arithop();
        void* rightNode = term();
        node = create_binary_expr_node(operation, node, rightNode);
    }
    return node;
}

void* term() {
    void* node = factor();
    while (lookahead == opMUL || lookahead == opDIV) {
        NodeType operation = arithop();
        void* rightNode = factor();
        node = create_binary_expr_node(operation, node, rightNode);
    }
    return node;
}

void* factor() {
    if (lookahead == LPAREN) {
        match(LPAREN);
        void* node = arith_exp();
        match(RPAREN);
        return node;
    }
    else if (lookahead == opSUB) {
        match(opSUB);
        UnaryExprNode* unaryNode = malloc(sizeof(UnaryExprNode));
        unaryNode->type = UMINUS;
        unaryNode->operand = factor();
        return unaryNode;
    }
    else {
        return simple();
    }
}

void* simple() {
    if (lookahead == ID) {
        char* id_name = strdup(lexeme);
        match(ID);
        if (lookahead == LPAREN) {
            return fn_call_or_assg();
        }
        if (check_variable_declaration(id_name) == -1) {
            fprintf(stderr, "ERROR: Use of undeclared variable '%s' on LINE %d\n", id_name, line_number);
            exit(1);
        }
        IdentifierNode* idNode = create_identifier_node(id_name);
        return idNode;
    }
    else {  // lookahead == INTCON
        int value = atoi(lexeme);
        match(INTCON);
        IntConstNode* intNode = create_int_const_node(value);
        return intNode;
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////


NodeType relop() {
    NodeType opType;
    if (lookahead == opEQ) opType = EQ;
    else if (lookahead == opNE) opType = NE;
    else if (lookahead == opLE) opType = LE;
    else if (lookahead == opLT) opType = LT;
    else if (lookahead == opGE) opType = GE;
    else if (lookahead == opGT) opType = GT;
    else {
        fprintf(stderr, "ERROR: Invalid relational operator on LINE %d\n", line_number);
        exit(1);
    }
    advance();
    return opType;
}


NodeType logical_op() {
    NodeType opType;
    if (lookahead == opAND) {
        match(opAND);
        return AND;
    }
    else if (lookahead == opOR) {
        match(opOR);
        return OR;
    }
    else {
        fprintf(stderr, "ERROR: Expected logical operator on LINE %d\n", line_number);
        exit(1);
    }
}


NodeType arithop() {
    NodeType opType;
    switch (lookahead) {
    case opADD:
        opType = ADD;
        match(opADD);
        break;
    case opSUB:
        opType = SUB;
        match(opSUB);
        break;
    case opMUL:
        opType = MUL;
        match(opMUL);
        break;
    case opDIV:
        opType = DIV;
        match(opDIV);
        break;
    default:
        fprintf(stderr, "ERROR: Expected arithmetic operator on LINE %d\n", line_number);
        exit(1);
    }
    return opType;
}
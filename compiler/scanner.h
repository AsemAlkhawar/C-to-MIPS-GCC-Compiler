#pragma once
#pragma once
#pragma once
#include "ast.h"
#include "gen_code.h"
#pragma warning(disable : 4996)
/*
 * File: scanner.h
 * Author: Ahmad Gaber
 * Purpose: Lists tokens and their values for use in the CSC 453 project
 */

#define MAX_IDENTIFIERS 10000
#define MAX_SCOPES 10000

typedef enum { GLOBAL, LOCAL } Scope;
typedef enum { VARIABLE, FUNCTION } IdType;

typedef struct {
	char* name;
	IdType type;
	int argCount;
} SymbolTableEntry;

typedef struct {
	SymbolTableEntry* entries; // Pointer to dynamically allocated array
	int size;
	int capacity; // Added to keep track of allocated memory size
} SymbolTable;

extern int currentScope;
extern SymbolTable symbolTableStack[MAX_SCOPES];

#ifndef __SCANNER_H__
#define __SCANNER_H__

void advance();
void match(int expectedToken);
int parse();
void prog();
void decl_or_func();
void id_list();
void type();
int opt_formals(char*** arg_names);
int formals(char*** arg_names);
void opt_var_decls();
StmtListNode* opt_stmt_list();
void* stmt();
IfNode* if_stmt();
WhileNode* while_stmt();
ReturnNode* return_stmt();
void* fn_call_or_assg();
ExprListNode* expr_list(int* arg_count);
BinaryExprNode* bool_exp();
BinaryExprNode* and_exp();
void* arith_exp();
NodeType relop();
NodeType arithop();
NodeType logical_op();

void* term();
void* factor();
void* simple();

int check_variable_declaration(char* name);
int check_function_call(char* name, int argCount);
int add_to_symbol_table(char* name, IdType type, Scope scope);
void pop_scope();
void push_scope();
int update_function_arg_count(char* name, int argCount);
BinaryExprNode* relational_exp();


FuncDefNode* create_func_def_node(char* name, int nargs, char** argnames, void* body);
FuncCallNode* create_func_call_node(char* callee, void* args);
IfNode* create_if_node(void* expr, void* then_branch, void* else_branch);
WhileNode* create_while_node(void* expr, void* body);
AssgNode* create_assg_node(char* lhs, void* rhs);
ReturnNode* create_return_node(void* expr);
StmtListNode* create_stmt_list_node(struct DummyNode* stmt, struct StmtListNode* next);
ExprListNode* create_expr_list_node(struct DummyNode* expr, struct ExprListNode* next);
IdentifierNode* create_identifier_node(char* name);
IntConstNode* create_int_const_node(int value);
BinaryExprNode* create_binary_expr_node(NodeType type, void* operand1, void* operand2);
UnaryExprNode* create_unary_expr_node(void* operand);
NodeType ast_node_type(void* ptr);


extern char* lexeme;
/*
 * The enum Token defines integer values for the various tokens.  These
 * are the values returned by the scanner.
 */
typedef enum {
	UNDEF     /* undefined */,
	ID        /* identifier: e.g., x, abc, p_q_12 */,
	INTCON    /* integer constant: e.g., 12345 */,
	LPAREN    /* '(' : Left parenthesis */,
	RPAREN    /* ')' : Right parenthesis */,
	LBRACE    /* '{' : Left curly brace */,
	RBRACE    /* '}' : Right curly brace */,
	COMMA     /* ',' : Comma */,
	SEMI      /*	;  : Semicolon */,
	kwINT     /*	int */,
	kwIF      /*	if */,
	kwELSE    /*	else */,
	kwWHILE   /*	while */,
	kwRETURN  /*	return */,
	opASSG    /*	= : Assignment */,
	opADD     /*	+ : addition */,
	opSUB     /*	  : subtraction */,
	opMUL     /*	* : multiplication */,
	opDIV     /*	/ : division */,
	opEQ      /*	== : Op: equals */,
	opNE      /*	!= : op: not-equals */,
	opGT      /*	>  : Op: greater-than */,
	opGE      /*	>= : Op: greater-or-equal */,
	opLT      /*	<  : Op: less-than */,
	opLE      /*	<= : Op: less-or-equal */,
	opAND     /*	&& : Op: logical-and */,
	opOR      /*	|| : Op: logical-or */,
	opNOT     /* ! : Op: logical-not */
} Token;

#endif  /* __SCANNER_H__ */
